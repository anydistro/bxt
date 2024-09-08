/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "persistence/box/record/PackageRecord.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

namespace bxt::Persistence::Box {

struct PoolBase {
    struct FsError : public bxt::Error {
        FsError(std::error_code const& ec) {
            message = ec.message();
        }
    };
    BXT_DECLARE_RESULT(FsError);

    virtual Result<PackageRecord> move_to(PackageRecord const& package) = 0;
    virtual Result<void> remove(PackageRecord const& package) = 0;

    virtual Result<PackageRecord> path_for_package(PackageRecord const& package) const = 0;
};
} // namespace bxt::Persistence::Box
