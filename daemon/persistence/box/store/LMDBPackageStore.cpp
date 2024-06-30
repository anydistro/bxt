/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "LMDBPackageStore.h"

#include "core/domain/repositories/UnitOfWorkBase.h"
#include "persistence/box/pool/PoolBase.h"
#include "persistence/box/record/PackageRecord.h"
#include "persistence/lmdb/LmdbUnitOfWork.h"

#include <filesystem>
#include <memory>
#include <string>

namespace bxt::Persistence::Box {

LMDBPackageStore::LMDBPackageStore(
    BoxOptions& box_options,
    std::shared_ptr<Utilities::LMDB::Environment> env,
    PoolBase& pool,
    const std::string_view name)
    : m_root_path(box_options.box_path), m_pool(pool), m_db(env, name) {
}
coro::task<std::expected<void, DatabaseError>>
    LMDBPackageStore::add(const PackageRecord package,
                          std::shared_ptr<UnitOfWorkBase> uow) {
    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(
            DatabaseError::ErrorType::InvalidArgument);
    }

    auto package_after_move = m_pool.path_for_package(package);

    if (!package_after_move.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(package_after_move.error()),
            DatabaseError::ErrorType::InvalidArgument);
    }

    const auto key = package.id.to_string();

    auto existing_package = co_await m_db.get(lmdb_uow->txn().value, key);
    if (existing_package.has_value()) {
        co_return bxt::make_error<DatabaseError>(
            DatabaseError::ErrorType::AlreadyExists);
    }

    auto result =
        co_await m_db.put(lmdb_uow->txn().value, key, *package_after_move);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(result.error()),
            DatabaseError::ErrorType::InvalidArgument);
    }

    lmdb_uow->hook([this, package = std::move(package)] {
        m_pool.move_to(std::move(package));
    });

    co_return {};
}

coro::task<std::expected<void, DatabaseError>>
    LMDBPackageStore::delete_by_id(const PackageRecord::Id package_id,
                                   std::shared_ptr<UnitOfWorkBase> uow) {
    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(
            DatabaseError::ErrorType::InvalidArgument);
    }

    auto package_to_delete =
        co_await m_db.get(lmdb_uow->txn().value, package_id.to_string());

    if (!package_to_delete.has_value()) {
        co_return bxt::make_error<DatabaseError>(
            DatabaseError::ErrorType::EntityNotFound);
    }

    auto result =
        co_await m_db.del(lmdb_uow->txn().value, package_id.to_string());

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(result.error()),
            DatabaseError::ErrorType::InvalidArgument);
    }
    lmdb_uow->hook([this, package_to_delete = std::move(*package_to_delete)] {
        return m_pool.remove(std::move(package_to_delete)).has_value();
    });

    co_return {};
}

coro::task<std::expected<void, DatabaseError>>
    LMDBPackageStore::update(const PackageRecord package,
                             std::shared_ptr<UnitOfWorkBase> uow) {
    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(
            DatabaseError::ErrorType::InvalidArgument);
    }
    auto moved_package = m_pool.move_to(package);

    if (!moved_package.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(moved_package.error()),
            DatabaseError::ErrorType::InvalidArgument);
    }

    const auto key = package.id.to_string();

    auto existing_package = co_await m_db.get(lmdb_uow->txn().value, key);
    if (existing_package.has_value()) {
        for (const auto& [location, desc] : moved_package->descriptions) {
            existing_package->descriptions[location] = desc;
        }
        moved_package = *existing_package;
    }

    auto result = co_await m_db.put(lmdb_uow->txn().value, key, *moved_package);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(result.error()),
            DatabaseError::ErrorType::InvalidArgument);
    }

    // auto mark = m_archiver.mark_dirty_sections({section});
    // co_await mark;

    co_return {};
}

coro::task<std::expected<std::vector<PackageRecord>, DatabaseError>>
    LMDBPackageStore::find_by_section(PackageSectionDTO section,
                                      std::shared_ptr<UnitOfWorkBase> uow) {
    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(
            DatabaseError::ErrorType::InvalidArgument);
    }

    std::vector<PackageRecord> result;
    co_await m_db.accept(
        lmdb_uow->txn().value,
        [&result]([[maybe_unused]] std::string_view key, const auto& value) {
            result.emplace_back(value);
            return Utilities::NavigationAction::Next;
        },
        std::string(section));

    co_return result;
}

coro::task<std::expected<void, DatabaseError>> LMDBPackageStore::accept(
    std::function<Utilities::NavigationAction(
        std::string_view key, const PackageRecord& value)> visitor,
    std::shared_ptr<UnitOfWorkBase> uow) {
    co_return co_await accept(visitor, "", uow);
}

coro::task<std::expected<void, DatabaseError>> LMDBPackageStore::accept(
    std::function<Utilities::NavigationAction(
        std::string_view key, const PackageRecord& value)> visitor,
    std::string_view prefix,
    std::shared_ptr<UnitOfWorkBase> uow) {
    auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
    if (!lmdb_uow) {
        co_return bxt::make_error<DatabaseError>(
            DatabaseError::ErrorType::InvalidArgument);
    }
    co_await m_db.accept(lmdb_uow->txn().value, visitor, prefix);

    co_return {};
}

} // namespace bxt::Persistence::Box
