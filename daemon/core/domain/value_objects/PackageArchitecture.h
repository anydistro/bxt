/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string>

namespace bxt::Core::Domain {

class PackageArchitecture {
public:
    PackageArchitecture(std::string const& arch_name = "");

    operator std::string const&() const {
        return m_arch_name;
    }

private:
    std::string m_arch_name;
};

} // namespace bxt::Core::Domain
