/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "parallel_hashmap/phmap.h"
#include "utilities/repo-schema/SchemaExtension.h"

#include <string>
#include <yaml-cpp/yaml.h>

namespace bxt::Persistence::Box {
struct PoolOptions : public Utilities::RepoSchema::Extension {
    phmap::flat_hash_map<std::string, std::string> templates;

    void parse(const YAML::Node& root_node) override {
        constexpr char Tag[] = "(box.pool)";
        const auto& options_node = root_node[Tag];

        for (const auto& repo : root_node["repositories"]) {
            const auto& key = repo.first;
            const auto& value = repo.second;
            if (!key || !value || !value.IsMap()) { continue; }

            const auto pool_options = value[Tag];

            const auto architecture = value["architecture"].as<std::string>();

            const auto template_string =
                pool_options["template"].as<std::string>();

            if (key.IsScalar()) {
                templates.emplace(key.as<std::string>(), template_string);
            } else if (key.IsSequence()) {
                for (const auto el : key) {
                    templates.emplace(el.as<std::string>(), template_string);
                }
            }
        }
    }
};
} // namespace bxt::Persistence::Box
