/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <chrono>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

#include <boost/algorithm/string/split.hpp>
#include <date/date.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <spdlog/details/log_msg.h>

namespace bxt::logging {

// Special character to separate structured data (Unit Separator)
constexpr char StructDataSeparator = '\x1F';

// Time unit constants
constexpr auto MicrosecondsPerMillisecond = 1000;
constexpr auto MillisecondsPerSecond = 1000;
constexpr auto SecondsPerMinute = 60;
constexpr int MinutesPerHour = 60;

struct ThreadTimeContext {
    std::chrono::steady_clock::time_point last_log_time = std::chrono::steady_clock::now();
};

struct StructuredDataItem {
    std::vector<std::string> lines;
};

// Parse a log message to separate the main message from structured data
inline std::pair<std::string, std::vector<StructuredDataItem>>
    parse_message(fmt::string_view const& payload) {
    std::string full_message = fmt::format("{}", payload);
    std::vector<StructuredDataItem> structured_data;
    std::string main_message;
    size_t separator_pos = full_message.find(StructDataSeparator);
    if (separator_pos != std::string::npos) {
        main_message = full_message.substr(0, separator_pos);

        std::string data_part = full_message.substr(separator_pos + 1);
        std::vector<std::string> split_data;
        boost::split(split_data, data_part, [](char c) { return c == StructDataSeparator; });

        for (auto const& data : split_data) {
            StructuredDataItem item;
            boost::split(item.lines, data, [](char c) { return c == '\n'; });
            structured_data.push_back(item);
        }
    } else {
        main_message = full_message;
    }
    return {main_message, structured_data};
}

// Format a timestamp from a log message

inline std::string format_timestamp(spdlog::details::log_msg const& msg) {
    auto time_point = msg.time;
    return date::format("%Y-%m-%d %H:%M:%S", time_point);
}

// Calculate adaptive elapsed time with appropriate units
inline std::string calculate_elapsed_time(ThreadTimeContext& ctx) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - ctx.last_log_time;
    ctx.last_log_time = now;

    using namespace std::chrono;

    auto format_duration = [](auto duration) -> std::string {
        if (duration < microseconds(MicrosecondsPerMillisecond)) {
            return fmt::format("+{:%Q%q}", duration_cast<microseconds>(duration));
        } else if (duration < milliseconds(MillisecondsPerSecond)) {
            return fmt::format("+{:%Q%q}", duration_cast<milliseconds>(duration));
        } else if (duration < seconds(SecondsPerMinute)) {
            return fmt::format("+{:.1f}s", std::chrono::duration<float>(duration).count());
        } else {
            // Custom formatting for readability in longer durations
            auto total_seconds = duration_cast<seconds>(duration).count();
            if (total_seconds < SecondsPerMinute * MinutesPerHour) {
                return fmt::format("+{}m:{:02d}s", total_seconds / SecondsPerMinute,
                                   total_seconds % SecondsPerMinute);
            } else {
                auto hours = total_seconds / (SecondsPerMinute * MinutesPerHour);
                auto minutes =
                    (total_seconds % (SecondsPerMinute * MinutesPerHour)) / SecondsPerMinute;
                auto secs = total_seconds % SecondsPerMinute;
                return fmt::format("+{}h:{}m:{:02d}s", hours, minutes, secs);
            }
        }
    };

    return fmt::format("{:>15}", format_duration(elapsed));
}
// Format source location from a log message

inline std::string format_source_location(spdlog::source_loc const& source) {
    std::string source_str;
    if (source.filename) {
        source_str = fmt::format(" [{}:{}]", source.filename, source.line);
        if (source.funcname) {
            source_str += fmt::format(" [{}]", source.funcname);
        }
    }
    return source_str;
}

using source_location = std::source_location;

[[nodiscard]] constexpr auto get_log_source_location(source_location const& location) {
    return spdlog::source_loc {location.file_name(), static_cast<std::int32_t>(location.line()),
                               location.function_name()};
}

struct format_with_location {
    std::string_view value;
    spdlog::source_loc loc;

    template<typename String>
    format_with_location(String const& s,
                         source_location const& location = source_location::current())
        : value {s}
        , loc {get_log_source_location(location)} {
    }
};
} // namespace bxt::logging
