/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

namespace bxt::Core::Domain {
class Permission {
public:
    Permission(std::string const& permission) {
        boost::split(m_permission_tags, permission, boost::is_any_of("."));
    }

    operator std::string const() const {
        return boost::join(m_permission_tags, ".");
    }

    std::vector<std::string> tags() const {
        return m_permission_tags;
    }

    auto operator<=>(Permission const& other) const = default;

private:
    std::vector<std::string> m_permission_tags;
};

} // namespace bxt::Core::Domain
