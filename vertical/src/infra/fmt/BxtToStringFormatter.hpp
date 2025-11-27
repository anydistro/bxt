/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <fmt/format.h>

#include "core/result_types.hpp"
#include "core/types/Path.hpp"
#include "core/types/TimePoint.hpp"
#include "core/types/to_string.hpp"

template<> struct fmt::formatter<> : fmt::formatter<std::string> {
    template<typename FormatCtx>
    auto format(bxt::Users::domain::Permission const& a, FormatCtx& ctx) const {
        return fmt::formatter<std::string>::format(bxt::to_string(a), ctx);
    }
};
