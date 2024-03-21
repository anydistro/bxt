/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <fmt/core.h>
#include <string>

namespace bxt {

inline boost::log::sources::severity_logger<boost::log::trivial::severity_level>
    logger;

inline void log(boost::log::trivial::severity_level level,
                const std::string& str) {
    BOOST_LOG_SEV(logger, level) << str;
}

template<typename... T>
inline void log(boost::log::trivial::severity_level level,
                fmt::format_string<T...> format,
                T&&... args) {
    BOOST_LOG_SEV(logger, level)
        << fmt::format(format, std::forward<T>(args)...);
}

// Trace
inline void logt(const std::string& string) {
    log(boost::log::trivial::trace, string);
}

template<typename... T>
inline void logt(fmt::format_string<T...> format, T&&... args) {
    log(boost::log::trivial::trace, format, std::forward<T>(args)...);
}

// Debug
inline void logd(const std::string& string) {
    log(boost::log::trivial::debug, string);
}

template<typename... T>
inline void logd(fmt::format_string<T...> format, T&&... args) {
    log(boost::log::trivial::debug, format, std::forward<T>(args)...);
}

// Info
inline void logi(const std::string& string) {
    log(boost::log::trivial::info, string);
}

template<typename... T>
inline void logi(fmt::format_string<T...> format, T&&... args) {
    log(boost::log::trivial::info, format, std::forward<T>(args)...);
}

// Warning
inline void logw(const std::string& string) {
    log(boost::log::trivial::warning, string);
}

template<typename... T>
inline void logw(fmt::format_string<T...> format, T&&... args) {
    log(boost::log::trivial::warning, format, std::forward<T>(args)...);
}

// Error
inline void loge(const std::string& string) {
    log(boost::log::trivial::error, string);
}

template<typename... T>
inline void loge(fmt::format_string<T...> format, T&&... args) {
    log(boost::log::trivial::error, format, std::forward<T>(args)...);
}

// Fatal
inline void logf(const std::string& string) {
    log(boost::log::trivial::fatal, string);
}

template<typename... T>
inline void logf(fmt::format_string<T...> format, T&&... args) {
    log(boost::log::trivial::fatal, format, std::forward<T>(args)...);
}

} // namespace bxt
