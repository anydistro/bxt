/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "LMDBPackageStore.h"

#include "core/domain/repositories/ReadOnlyRepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "persistence/box/pool/PoolBase.h"
#include "persistence/box/record/PackageRecord.h"
#include "persistence/lmdb/LmdbUnitOfWork.h"
#include "utilities/to_string.h"

#include <filesystem>
#include <memory>
#include <string>

namespace bxt::Persistence::Box {

LMDBPackageStore::LMDBPackageStore(BoxOptions& box_options,
                                   std::shared_ptr<Utilities::LMDB::Environment> env,
                                   PoolBase& pool,
                                   ReadOnlyRepositoryBase<Section>& section_repository,
                                   std::string_view const name)
    : m_root_path(box_options.box_path)
    , m_pool(pool)
    , m_db(env, name)
    , m_section_repository(section_repository) {
}

coro::task<std::expected<void, DatabaseError>>
    LMDBPackageStore::add(PackageRecord const package, std::shared_ptr<UnitOfWorkBase> uow) {
    auto section =
        co_await m_section_repository.find_by_id_async(bxt::to_string(package.id.section), uow);

    if (!section.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(section.error()), DatabaseError::ErrorType::InvalidArgument);
    }

    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(DatabaseError::ErrorType::InvalidArgument);
    }

    auto package_after_move = m_pool.path_for_package(package);

    if (!package_after_move.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(package_after_move.error()), DatabaseError::ErrorType::InvalidArgument);
    }

    auto const key = package.id.to_string();

    auto existing_package = co_await m_db.get(lmdb_uow->txn().value, key);
    if (existing_package.has_value()) {
        co_return bxt::make_error<DatabaseError>(DatabaseError::ErrorType::AlreadyExists);
    }

    auto result = co_await m_db.put(lmdb_uow->txn().value, key, *package_after_move);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(result.error()), DatabaseError::ErrorType::InvalidArgument);
    }

    lmdb_uow->hook([this, package = std::move(package)] { m_pool.move_to(std::move(package)); });

    co_return {};
}

coro::task<std::expected<void, DatabaseError>>
    LMDBPackageStore::delete_by_id(PackageRecord::Id const package_id,
                                   std::shared_ptr<UnitOfWorkBase> uow) {
    auto section =
        co_await m_section_repository.find_by_id_async(bxt::to_string(package_id.section), uow);

    if (!section.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(section.error()), DatabaseError::ErrorType::InvalidArgument);
    }

    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(DatabaseError::ErrorType::InvalidArgument);
    }

    auto package_to_delete = co_await m_db.get(lmdb_uow->txn().value, package_id.to_string());

    if (!package_to_delete.has_value()) {
        co_return bxt::make_error<DatabaseError>(DatabaseError::ErrorType::EntityNotFound);
    }

    auto result = co_await m_db.del(lmdb_uow->txn().value, package_id.to_string());

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(result.error()), DatabaseError::ErrorType::InvalidArgument);
    }
    lmdb_uow->hook([this, package_to_delete = std::move(*package_to_delete)] {
        return m_pool.remove(std::move(package_to_delete)).has_value();
    });

    co_return {};
}

coro::task<std::expected<void, DatabaseError>>
    LMDBPackageStore::update(PackageRecord const package, std::shared_ptr<UnitOfWorkBase> uow) {
    auto section =
        co_await m_section_repository.find_by_id_async(bxt::to_string(package.id.section), uow);

    if (!section.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(section.error()), DatabaseError::ErrorType::InvalidArgument);
    }

    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(DatabaseError::ErrorType::InvalidArgument);
    }
    auto moved_package_path = m_pool.path_for_package(package);

    if (!moved_package_path.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(moved_package_path.error()), DatabaseError::ErrorType::InvalidArgument);
    }

    auto const key = package.id.to_string();

    auto existing_package = co_await m_db.get(lmdb_uow->txn().value, key);
    if (existing_package.has_value()) {
        auto merged_package = *existing_package;
        for (auto const& [location, desc] : moved_package_path->descriptions) {
            merged_package.descriptions[location] = desc;
        }
        moved_package_path = merged_package;
    } else {
        co_return bxt::make_error<DatabaseError>(DatabaseError::ErrorType::EntityNotFound);
    }

    auto result = co_await m_db.put(lmdb_uow->txn().value, key, *moved_package_path);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(result.error()), DatabaseError::ErrorType::InvalidArgument);
    }

    lmdb_uow->hook([this, package, moved_package_path, existing_package] {
        auto tmp_package = package;
        for (auto const& desc : moved_package_path->descriptions) {
            if (package.descriptions.contains(desc.first)
                && existing_package->descriptions.contains(desc.first)
                && desc.second.filepath == existing_package->descriptions.at(desc.first).filepath) {
                tmp_package.descriptions.erase(desc.first);
            }
        }

        m_pool.move_to(std::move(tmp_package));
    });

    co_return {};
}

coro::task<std::expected<std::vector<PackageRecord>, DatabaseError>>
    LMDBPackageStore::find_by_section(PackageSectionDTO section,
                                      std::shared_ptr<UnitOfWorkBase> uow) {
    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(DatabaseError::ErrorType::InvalidArgument);
    }

    std::vector<PackageRecord> result;
    co_await m_db.accept(
        lmdb_uow->txn().value,
        [&result]([[maybe_unused]] std::string_view key, auto const& value) {
            result.emplace_back(value);
            return Utilities::NavigationAction::Next;
        },
        std::string(section));

    co_return result;
}

coro::task<std::expected<void, DatabaseError>> LMDBPackageStore::accept(
    std::function<Utilities::NavigationAction(std::string_view key, PackageRecord const& value)>
        visitor,
    std::shared_ptr<UnitOfWorkBase> uow) {
    co_return co_await accept(visitor, "", uow);
}

coro::task<std::expected<void, DatabaseError>> LMDBPackageStore::accept(
    std::function<Utilities::NavigationAction(std::string_view key, PackageRecord const& value)>
        visitor,
    std::string_view prefix,
    std::shared_ptr<UnitOfWorkBase> uow) {
    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(DatabaseError::ErrorType::InvalidArgument);
    }
    co_await m_db.accept(lmdb_uow->txn().value, visitor, prefix);

    co_return {};
}

} // namespace bxt::Persistence::Box
