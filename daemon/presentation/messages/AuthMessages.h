/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string>

namespace bxt::Presentation {
struct AuthRequest {
    std::string name;
    std::string password;
    std::string response_type;
};

// for CLI usage
struct AuthResponse {
    std::string access_token;
    std::string refresh_token;
    std::string token_type;
};
} // namespace bxt::Presentation
