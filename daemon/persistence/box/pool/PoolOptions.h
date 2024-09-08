/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
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
        auto const& options_node = root_node[Tag];

        for (auto const& repo : root_node["repositories"]) {
            auto const& key = repo.first;
            auto const& value = repo.second;
            if (!key || !value || !value.IsMap()) {
                continue;
            }

            auto const pool_options = value[Tag];

            auto const architecture = value["architecture"].as<std::string>();

            auto const template_string = pool_options["template"].as<std::string>();

            templates.emplace(architecture, template_string);
        }
    }
};
} // namespace bxt::Persistence::Box
