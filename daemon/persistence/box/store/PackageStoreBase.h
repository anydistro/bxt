/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "persistence/box/record/PackageRecord.h"
#include "utilities/errors/DatabaseError.h"
#include "utilities/errors/Macro.h"
#include "utilities/NavigationAction.h"

#include <coro/task.hpp>

namespace bxt::Persistence::Box {
struct PackageStoreBase {
    virtual ~PackageStoreBase() = default;

    virtual coro::task<std::expected<void, DatabaseError>>
        add(PackageRecord const package, std::shared_ptr<UnitOfWorkBase> uow) = 0;

    virtual coro::task<std::expected<void, DatabaseError>>
        update(PackageRecord const package, std::shared_ptr<UnitOfWorkBase> uow) = 0;

    virtual coro::task<std::expected<void, DatabaseError>>
        delete_by_id(PackageRecord::Id const package_id, std::shared_ptr<UnitOfWorkBase> uow) = 0;

    virtual coro::task<std::expected<std::vector<PackageRecord>, DatabaseError>>
        find_by_section(PackageSectionDTO section, std::shared_ptr<UnitOfWorkBase> uow) = 0;

    virtual coro::task<std::expected<void, DatabaseError>> accept(
        std::function<Utilities::NavigationAction(std::string_view key, PackageRecord const& value)>
            visitor,
        std::string_view prefix,
        std::shared_ptr<UnitOfWorkBase> uow) = 0;

    virtual coro::task<std::expected<void, DatabaseError>> accept(
        std::function<Utilities::NavigationAction(std::string_view key, PackageRecord const& value)>
            visitor,
        std::shared_ptr<UnitOfWorkBase> uow) = 0;
};
} // namespace bxt::Persistence::Box
