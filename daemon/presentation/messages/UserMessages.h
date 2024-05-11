/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <optional>
#include <set>
#include <string>

namespace bxt::Presentation {
struct AddUserRequest {
    std::string name;
    std::string password;
    std::optional<std::set<std::string>> permissions;
};

struct UpdateUserRequest {
    std::string name;
    std::optional<std::string> password;
    std::optional<std::set<std::string>> permissions;
};

struct RemoveUserRequest {
    std::string name;
};

struct UserResponse {
    std::string name;
    std::set<std::string> permissions;
};
} // namespace bxt::Presentation
