/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>

namespace fmt {
inline std::string format(const std::string& format_string) {
    return format_string;
}
} // namespace fmt

#define bxtlog(level, _fmt, ...) \
    BOOST_LOG_TRIVIAL(level) << fmt::format(_fmt, ##__VA_ARGS__);

#define logt(_fmt, ...) bxtlog(trace, _fmt, ##__VA_ARGS__)
#define logd(_fmt, ...) bxtlog(debug, _fmt, ##__VA_ARGS__)
#define logi(_fmt, ...) bxtlog(info, _fmt, ##__VA_ARGS__)
#define logw(_fmt, ...) bxtlog(warning, _fmt, ##__VA_ARGS__)
#define loge(_fmt, ...) bxtlog(error, _fmt, ##__VA_ARGS__)
#define logf(_fmt, ...) bxtlog(fatal, _fmt, ##__VA_ARGS__)
