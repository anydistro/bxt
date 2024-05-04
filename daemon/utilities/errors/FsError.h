/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "utilities/Error.h"

#include <system_error>
namespace bxt {
struct FsError : public bxt::Error {
    FsError(const std::error_code& ec) { message = ec.message(); }
};
} // namespace bxt
