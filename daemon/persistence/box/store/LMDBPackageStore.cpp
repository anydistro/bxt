/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "LMDBPackageStore.h"

#include "persistence/box/pool/PoolBase.h"

#include <filesystem>
#include <set>
#include <string>

namespace bxt::Persistence::Box {

LMDBPackageStore::LMDBPackageStore(
    std::shared_ptr<Utilities::LMDB::Environment> env,
    PoolBase& pool,
    const std::filesystem::path& path,
    const std::string_view name)
    : m_root_path(std::move(path)), m_pool(pool), m_db(env, name) {
}

coro::task<nonstd::expected<void, DatabaseError>>
    LMDBPackageStore::add(const PackageRecord package) {
    auto moved_package = m_pool.move_to(package);

    if (!moved_package.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(moved_package.error()),
            DatabaseError::ErrorType::InvalidArgument);
    }

    const auto key = package.id.to_string();

    auto existing_package = co_await m_db.get(key);
    if (existing_package.has_value()) {
        for (const auto& [location, desc] : moved_package->descriptions) {
            existing_package->descriptions[location] = desc;
        }
        moved_package = *existing_package;
    }

    auto result = co_await m_db.put(key, *moved_package);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(result.error()),
            DatabaseError::ErrorType::InvalidArgument);
    }

    co_return {};
}

coro::task<nonstd::expected<void, DatabaseError>>
    LMDBPackageStore::remove(const PackageRecord::Id package_id) {
    auto result = co_await m_db.del(package_id.to_string());

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(result.error()),
            DatabaseError::ErrorType::InvalidArgument);
    }

    // auto mark = m_archiver.mark_dirty_sections({section});
    // co_await mark;

    co_return {};
}

coro::task<nonstd::expected<std::vector<PackageRecord>, DatabaseError>>
    LMDBPackageStore::find_by_section(PackageSectionDTO section) {
    std::vector<PackageRecord> result;

    co_await m_db.accept(
        [&result]([[maybe_unused]] std::string_view key, const auto& value) {
            result.emplace_back(value);
            return Utilities::NavigationAction::Next;
        },
        std::string(section));

    co_return result;
}
} // namespace bxt::Persistence::Box