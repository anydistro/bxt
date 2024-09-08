/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <chrono>
#include <date/date.h>
#include <iterator>
#include <rfl/parsing/CustomParser.hpp>
#include <rfl/Result.hpp>
#include <sstream>
#include <string>

/**
 * @class Iso8601TimePoint
 * @brief Class for parsing and formatting ISO 8601 time points.
 *
 * This class provides functionality to parse ISO 8601 formatted strings into
 * time_point objects and format time_point objects back into ISO 8601 formatted
 * strings. It is necessary to reflect time_point to string in the reflect-cpp
 * library.
 *
 * For more details on custom class reflection, see:
 * https://github.com/getml/reflect-cpp/blob/main/docs/custom_classes.md
 *
 * For more information on custom parsers, see:
 * https://github.com/getml/reflect-cpp/blob/main/docs/custom_parser.md
 */
class Iso8601TimePoint {
    static constexpr auto const time_format = "%Y-%m-%dT%H:%M:%SZ";

public:
    using ReflectionType = std::string;
    using time_point = std::chrono::system_clock::time_point;

    Iso8601TimePoint(char const* _str) {
        if (!_str) {
            throw std::invalid_argument("Cannot parse null string");
        }
        std::istringstream ss(_str);
        if (ss.str().empty()) {
            throw std::invalid_argument("Cannot parse empty string");
        }

        date::from_stream(ss, time_format, m_time_point);
    }

    Iso8601TimePoint(std::string const& _str)
        : Iso8601TimePoint(_str.c_str()) {
    }

    Iso8601TimePoint(time_point const& tp)
        : m_time_point(tp) {
    }

    ~Iso8601TimePoint() = default;

    static rfl::Result<Iso8601TimePoint> from_string(char const* _str) noexcept {
        try {
            return Iso8601TimePoint(_str);
        } catch (std::exception& e) {
            return rfl::Error(e.what());
        }
    }

    static rfl::Result<Iso8601TimePoint> from_string(std::string const& _str) {
        return from_string(_str.c_str());
    }

    ReflectionType reflection() const {
        return date::format(time_format,
                            std::chrono::floor<std::chrono::milliseconds>(m_time_point));
    }

    std::string str() const {
        return reflection();
    }

    time_point to_class() const {
        return m_time_point;
    }

    static Iso8601TimePoint from_class(time_point const& tp) {
        return {tp};
    }

private:
    time_point m_time_point;
};

template<class ReaderType, class WriterType, class ProcessorsType>
struct rfl::parsing::
    Parser<ReaderType, WriterType, std::chrono::system_clock::time_point, ProcessorsType>
    : public CustomParser<ReaderType,
                          WriterType,
                          ProcessorsType,
                          std::chrono::system_clock::time_point,
                          Iso8601TimePoint> {};
