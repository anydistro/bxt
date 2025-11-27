/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <source_location>
#include <string>
#include <type_traits>

#include <fmt/core.h>
#include <rfl/json/write.hpp>
#include <rfl/to_view.hpp>
#include <spdlog/spdlog.h>

#include "FormattingUtils.hpp"

namespace bxt {

// Base logging function for structured data
template<typename... Args>
inline void log_structured(spdlog::level::level_enum level,
                           logging::format_with_location const& fmt,
                           Args&&... args) {
    auto formatted_message = std::string(fmt.value);

    (
        [&] {
            try {
                std::string data_str;
                if constexpr (std::is_convertible_v<Args, std::string>) {
                    data_str = std::forward<Args>(args);
                } else if constexpr (requires { fmt::format("{}", std::declval<Args>()); }) {
                    data_str = fmt::format("{}", std::forward<Args>(args));
                } else {
                    data_str = rfl::json::write(std::forward<Args>(args)..., rfl::json::pretty);
                }

                formatted_message += logging::StructDataSeparator + data_str;
            } catch (std::exception const& e) {
                formatted_message += logging::StructDataSeparator
                                     + fmt::format("<Error formatting value: {}>", e.what());
            }
        }(),
        ...);

    spdlog::default_logger_raw()->log(fmt.loc, level, formatted_message);
}

// Trace level with structured data
template<typename... Args>
inline void logt_s(logging::format_with_location const& data, Args&&... args) {
    log_structured(spdlog::level::trace, data, std::forward<Args>(args)...);
}
// Debug level with structured data
template<typename... Args>
inline void logd_s(logging::format_with_location const& data, Args&&... args) {
    log_structured(spdlog::level::debug, data, std::forward<Args>(args)...);
}

// Info level with structured data
template<typename... Args>
inline void logi_s(logging::format_with_location const& data, Args&&... args) {
    log_structured(spdlog::level::info, data, std::forward<Args>(args)...);
}

// Warning level with structured data
template<typename... Args>
inline void logw_s(logging::format_with_location const& data, Args&&... args) {
    log_structured(spdlog::level::warn, data, std::forward<Args>(args)...);
}

// Error level with structured data
template<typename... Args>
inline void loge_s(logging::format_with_location const& data, Args&&... args) {
    log_structured(spdlog::level::err, data, std::forward<Args>(args)...);
}

// Fatal level with structured data
template<typename... Args>
inline void logf_s(logging::format_with_location const& data, Args&&... args) {
    log_structured(spdlog::level::critical, data, std::forward<Args>(args)...);
}
} // namespace bxt
