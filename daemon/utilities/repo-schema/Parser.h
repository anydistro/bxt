/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "utilities/repo-schema/SchemaExtension.h"

#include <filesystem>
#include <parallel_hashmap/phmap.h>
#include <string>
#include <vector>

namespace bxt::Utilities::RepoSchema {

class Parser {
public:
    Parser() = default;

    phmap::flat_hash_set<PackageSectionDTO> sections() const {
        return m_sections;
    }

    void extend(Extension* extension);
    void parse(const std::filesystem::path& filename);

private:
    void parse(const YAML::Node& root_node);
    phmap::flat_hash_set<Extension*> m_extensions;

    phmap::flat_hash_set<PackageSectionDTO> m_sections;
};

} // namespace bxt::Utilities::RepoSchema
