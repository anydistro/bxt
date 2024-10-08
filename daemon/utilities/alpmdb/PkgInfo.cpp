/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PkgInfo.h"

#include <algorithm>
#include <boost/algorithm/string/regex.hpp>
#include <iostream>
#include <regex>
#include <sstream>

namespace bxt::Utilities::AlpmDb {

void PkgInfo::parse(std::string_view contents) {
    std::string_view line;
    std::size_t pos = 0, prev_pos = 0;

    while ((pos = contents.find('\n', prev_pos)) != std::string_view::npos) {
        line = contents.substr(prev_pos, pos - prev_pos);
        prev_pos = pos + 1;

        if (line.starts_with("#")) {
            continue;
        }

        std::size_t delim_pos = line.find(" = ");
        if (delim_pos == std::string_view::npos) {
            continue;
        }

        std::string_view key = line.substr(0, delim_pos);
        std::string_view value = line.substr(delim_pos + 3);

        m_values[std::string(key)].emplace_back(value);
    }
}

std::vector<std::string> PkgInfo::values(std::string const& key) const {
    auto it = m_values.find(key);
    if (it != m_values.end()) {
        return it->second;
    }
    return {};
}

} // namespace bxt::Utilities::AlpmDb
