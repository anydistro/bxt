/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 */
#pragma once

#include <chrono>
#include <sstream>
#include <string>

#include <date/date.h>

#include "infra/reflect/StringReflector.hpp"
#include "core/result_types.hpp"
#include "core/error/kinds.hpp"

/**
 * @file TimePoint.hpp
 * @brief ISO 8601 time point handling and reflect-cpp integration.
 *
 * This file provides a TimePoint type alias and string conversion functions
 * for parsing and formatting ISO 8601 time strings (YYYY-MM-DDTHH:MM:SSZ).
 * Includes reflect-cpp integration for automatic serialization/deserialization.
 *
 * Usage:
 * - Parse: auto result = bxt::from_string<TimePoint>("2024-01-15T14:30:45Z");
 * - Format: std::string iso = bxt::to_string(time_point)

 * @see https://github.com/getml/reflect-cpp/blob/main/docs/custom_classes.md
 * @see https://github.com/getml/reflect-cpp/blob/main/docs/custom_parser.md
 */

namespace bxt {

constexpr auto const TimeFormat = "%Y-%m-%dT%H:%M:%SZ";

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

template<>
inline Result<TimePoint, err::ParseError> from_string<TimePoint>(std::string_view str) {
    using err::ParseError;

    if (str.empty()) {
        return cpperr::make_error(ParseError::InvalidFormat,
                                  "Cannot parse TimePoint from empty string");
    }

    std::istringstream stream((std::string(str)));
    TimePoint time_point;

    date::from_stream(stream, TimeFormat, time_point);

    if (stream.fail()) {
        return cpperr::make_error(ParseError::InvalidFormat,
                                  "Failed to parse TimePoint from string: '"
                                      + std::string(str)
                                      + "'. "
                                        "Expected format: "
                                      + TimeFormat);
    }

    return time_point;
}

template<> inline std::string to_string(TimePoint const& time_point) {
    std::ostringstream stream;
    date::to_stream(stream, TimeFormat, time_point);
    return stream.str();
}

} // namespace bxt

BXT_REFLECT_DECLARE_STRING_REFLECTOR(bxt::TimePoint)
