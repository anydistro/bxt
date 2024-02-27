/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "persistence/box/record/PackageRecord.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

namespace bxt::Persistence::Box {

struct PoolBase {
    struct FsError : public bxt::Error {
        FsError(const std::error_code& ec) { message = ec.message(); }
    };
    BXT_DECLARE_RESULT(FsError);

    virtual Result<PackageRecord> move_to(const PackageRecord& package) = 0;
};
} // namespace bxt::Persistence::Box