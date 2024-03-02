/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "Parser.h"

#include <yaml-cpp/yaml.h>

namespace bxt::Utilities::RepoSchema {

void Parser::extend(Extension* extension) {
    m_extensions.emplace(extension);
}

void Parser::parse(const std::filesystem::path& filename) {
    parse(YAML::LoadFile(filename));
}

void Parser::parse(const YAML::Node& root_node) {
    if (!root_node || !root_node.IsMap()) {
        throw std::invalid_argument("Root node is malformed");
    }

    for (auto branch : root_node["branches"].as<std::vector<std::string>>()) {
        for (const auto& repo : root_node["repositories"]) {
            const auto& key = repo.first;
            const auto& value = repo.second;
            if (!key || !value || !value.IsMap()) { continue; }

            auto architecture = value["architecture"].as<std::string>();

            if (key.IsScalar()) {
                m_sections.emplace(branch, key.as<std::string>(), architecture);
            } else if (key.IsSequence()) {
                for (const auto& repository : key) {
                    m_sections.emplace(branch, repository.as<std::string>(),
                                       architecture);
                }
            }
        }
    }

    for (const auto& extension : m_extensions) {
        if (!extension) { return; }
        extension->parse(root_node);
    }
}

} // namespace bxt::Utilities::RepoSchema
