/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "utilities/repo-schema/SchemaExtension.h"

#include <parallel_hashmap/phmap.h>

namespace bxt::Infrastructure {

class PackageServiceOptions : public Utilities::RepoSchema::Extension {
public:
    std::filesystem::path
        pool(const Core::Application::PackageSectionDTO& section) const;

    virtual void parse(const YAML::Node& root_node) override;

private:
    phmap::flat_hash_map<Core::Application::PackageSectionDTO,
                         std::filesystem::path>
        m_pool_path_overrides;

    std::filesystem::path m_default_pool_path = "box/pool/overlay/";
};

} // namespace bxt::Infrastructure
