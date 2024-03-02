/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/Error.h"

#include <frozen/unordered_map.h>

namespace bxt::Core::Application {

struct AuthError : public bxt::Error {
    enum class ErrorType { InvalidCredentials, UserNotFound, InternalError };

    AuthError(ErrorType error_type) : error_type(error_type) {
        message = error_strings.at(error_type).data();
    }

    ErrorType error_type;

private:
    static inline frozen::unordered_map<ErrorType, std::string, 3>
        error_strings = {{ErrorType::InvalidCredentials, "Invalid credentials"},
                         {ErrorType::UserNotFound, "User not found"},
                         {ErrorType::InternalError, "Internal error"}};
};

} // namespace bxt::Core::Application
