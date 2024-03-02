/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "frozen/string.h"
#include "utilities/Error.h"

#include <frozen/unordered_map.h>

namespace bxt::Core::Application {
struct CompareError : public bxt::Error {
    enum class ErrorType {

        InvalidArgument,
        InternalError
    };

    CompareError(ErrorType error_type) : error_type(error_type) {
        message = error_strings.at(error_type).data();
    }

    ErrorType error_type;

private:
    static inline frozen::unordered_map<ErrorType, frozen::string, 2>
        error_strings = {{ErrorType::InvalidArgument, "Invalid argument."},
                         {ErrorType::InternalError, "Internal error."}};
};

} // namespace bxt::Core::Application
