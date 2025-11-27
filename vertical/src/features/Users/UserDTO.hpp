/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <optional>
#include <set>
#include <string>

namespace bxt::Users {

struct UserDTO {
    std::string name;
    std::optional<std::string> password;
    std::optional<std::set<std::string>> permissions;
};

} // namespace bxt::Users
