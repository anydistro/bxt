/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

// Test types for string reflection
#include <string>

#include "shared/common.hpp"
#include "shared/common/ToString.hpp"

struct SimpleTestType {
    std::string value;

    bool operator==(SimpleTestType const& other) const {
        return value == other.value;
    }
};

struct ComplexTestType {
    int number;
    std::string text;

    bool operator==(ComplexTestType const& other) const {
        return number == other.number && text == other.text;
    }
};

struct ErrorTestType {
    std::string value;

    bool operator==(ErrorTestType const& other) const {
        return value == other.value;
    }
};

// Implement string conversion functions
namespace bxt {
template<> inline std::string to_string(SimpleTestType const& value) {
    return value.value;
}

template<> inline Result<SimpleTestType, err::ParseError> from_string(std::string_view str) {
    return SimpleTestType {std::string(str)};
}

template<> inline std::string to_string(ComplexTestType const& value) {
    return std::to_string(value.number) + ":" + value.text;
}

template<> inline Result<ComplexTestType, err::ParseError> from_string(std::string_view str) {
    auto pos = str.find(':');
    if (pos == std::string_view::npos) {
        return cpperr::make_error(err::ParseError::InvalidFormat,
                                  "Invalid format for ComplexTestType");
    }

    try {
        int number = std::stoi(std::string(str.substr(0, pos)));
        std::string text = std::string(str.substr(pos + 1));
        return ComplexTestType {number, text};
    } catch (std::exception const&) {
        return cpperr::make_error(err::ParseError::InvalidValue,
                                  "Failed to parse ComplexTestType");
    }
}

template<> inline std::string to_string(ErrorTestType const& value) {
    return value.value;
}

template<> inline Result<ErrorTestType, err::ParseError> from_string(std::string_view str) {
    if (str == "error") {
        return cpperr::make_error(err::ParseError::InvalidValue,
                                  "Intentional error for testing");
    }
    return ErrorTestType {std::string(str)};
}
} // namespace bxt

// Declare reflectors using the macro
BXT_REFLECT_DECLARE_STRING_REFLECTOR(SimpleTestType)
BXT_REFLECT_DECLARE_STRING_REFLECTOR(ComplexTestType)
BXT_REFLECT_DECLARE_STRING_REFLECTOR(ErrorTestType)
