/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PackageArchitecture.h"

namespace bxt::Core::Domain {

PackageArchitecture::PackageArchitecture(const std::string &arch_name)
    : m_arch_name(arch_name) {
    if (m_arch_name.empty()) { m_arch_name = "any"; }
}

} // namespace bxt::Core::Domain
