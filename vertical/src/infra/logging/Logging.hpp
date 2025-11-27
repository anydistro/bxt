/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "FormattingUtils.hpp"

namespace bxt {

// Base logging function with source location
inline void log(spdlog::level::level_enum level, logging::format_with_location fmt) {
    spdlog::default_logger_raw()->log(fmt.loc, level, fmt::runtime(fmt.value));
}

// Variadic template version for formatted logging with source location
template<typename... T>
inline void log(spdlog::level::level_enum level, logging::format_with_location fmt, T&&... args) {
    spdlog::default_logger_raw()->log(fmt.loc, level, fmt::runtime(fmt.value),
                                      std::forward<T>(args)...);
}

// Trace level with source location
inline void logt(logging::format_with_location fmt) {
    log(spdlog::level::trace, fmt);
}

template<typename... T> inline void logt(logging::format_with_location fmt, T&&... args) {
    log(spdlog::level::trace, fmt, std::forward<T>(args)...);
}

// Debug level with source location
inline void logd(logging::format_with_location fmt) {
    log(spdlog::level::debug, fmt);
}

template<typename... T> inline void logd(logging::format_with_location fmt, T&&... args) {
    log(spdlog::level::debug, fmt, std::forward<T>(args)...);
}

// Info level with source location
inline void logi(logging::format_with_location fmt) {
    log(spdlog::level::info, fmt);
}

template<typename... T> inline void logi(logging::format_with_location fmt, T&&... args) {
    log(spdlog::level::info, fmt, std::forward<T>(args)...);
}

// Warning level with source location
inline void logw(logging::format_with_location fmt) {
    log(spdlog::level::warn, fmt);
}

template<typename... T> inline void logw(logging::format_with_location fmt, T&&... args) {
    log(spdlog::level::warn, fmt, std::forward<T>(args)...);
}

// Error level with source location
inline void loge(logging::format_with_location fmt) {
    log(spdlog::level::err, fmt);
}

template<typename... T> inline void loge(logging::format_with_location fmt, T&&... args) {
    log(spdlog::level::err, fmt, std::forward<T>(args)...);
}

// Fatal level with source location
inline void logf(logging::format_with_location fmt) {
    log(spdlog::level::critical, fmt);
}

template<typename... T> inline void logf(logging::format_with_location fmt, T&&... args) {
    log(spdlog::level::critical, fmt, std::forward<T>(args)...);
}

} // namespace bxt
