/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include <string>
#include <unordered_map>

namespace bxt::Core::Application {
struct RequestContext {
    std::string user_name;
    std::unordered_map<std::string, std::string> claims;
};
} // namespace bxt::Core::Application
