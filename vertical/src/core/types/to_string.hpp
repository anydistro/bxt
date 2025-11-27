/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string>
#include <string_view>

#include "core/result_types.hpp"
#include "core/error/kinds.hpp"

namespace bxt {

template<typename T> std::string to_string(T const& value) {
    if constexpr (std::is_same_v<T, std::string>) {
        return value;
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(value);
    } else {
        return std::string(value);
    }
}

template<typename T> Result<T, err::ParseError> from_string(std::string_view str) {
    if constexpr (std::is_same_v<T, std::string>) {
        return str;
    } else {
        return T(str);
    }
}

template<typename T> Result<T, err::ParseError> from_string(std::string const& str) {
    return from_string<T>(std::string_view(str));
}

template<typename T>
concept StringConvertible = requires(T const& t, std::string_view sv) {
    { to_string(t) } -> std::same_as<std::string>;
    { from_string<T>(sv) } -> std::same_as<Result<T, err::ParseError>>;
};

} // namespace bxt
