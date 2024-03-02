/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PackageServiceOptions.h"

namespace bxt::Infrastructure {

std::filesystem::path
    PackageServiceOptions::pool(const PackageSectionDTO &section) const {
    if (m_pool_path_overrides.contains(section)) {
        return m_pool_path_overrides.at(section);
    }
    return m_default_pool_path;
}

void PackageServiceOptions::parse(const YAML::Node &root_node) {
}

} // namespace bxt::Infrastructure
