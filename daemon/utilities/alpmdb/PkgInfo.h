/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <set>
#include <string>
#include <unordered_map>

namespace bxt::Utilities::AlpmDb {

class PkgInfo {
public:
    PkgInfo() = default;
    void parse(std::string_view contents);

    std::set<std::string> values(const std::string& key);

private:
    std::unordered_multimap<std::string, std::string> m_values;
};

} // namespace bxt::Utilities::AlpmDb
