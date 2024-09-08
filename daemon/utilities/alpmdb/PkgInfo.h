/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace bxt::Utilities::AlpmDb {

class PkgInfo {
public:
    PkgInfo() = default;
    void parse(std::string_view contents);

    std::vector<std::string> values(std::string const& key) const;

private:
    std::unordered_map<std::string, std::vector<std::string>> m_values;
};

} // namespace bxt::Utilities::AlpmDb
