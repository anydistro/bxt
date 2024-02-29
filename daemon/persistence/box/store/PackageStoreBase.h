/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "persistence/box/record/PackageRecord.h"
#include "utilities/NavigationAction.h"
#include "utilities/errors/DatabaseError.h"
#include "utilities/errors/Macro.h"

#include <coro/task.hpp>

namespace bxt::Persistence::Box {
struct PackageStoreBase {
    virtual ~PackageStoreBase() = default;

    virtual coro::task<nonstd::expected<void, DatabaseError>>
        add(const PackageRecord package) = 0;

    virtual coro::task<nonstd::expected<void, DatabaseError>>
        remove(const PackageRecord::Id package_id) = 0;

    virtual coro::task<
        nonstd::expected<std::vector<PackageRecord>, DatabaseError>>
        find_by_section(PackageSectionDTO section) = 0;

    virtual coro::task<void>
        accept(std::function<Utilities::NavigationAction(
                   std::string_view key, const PackageRecord& value)> visitor,
               std::string_view prefix = "") = 0;
};
} // namespace bxt::Persistence::Box
