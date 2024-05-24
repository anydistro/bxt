/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <parallel_hashmap/phmap.h>
#include <string>
#include <yaml-cpp/yaml.h>

namespace bxt::Infrastructure {
struct ArchRepoSource {
    static ArchRepoSource from_node(const YAML::Node& node) {
        if (!node.IsDefined()) { return ArchRepoSource {}; }

        ArchRepoSource result;
        auto get_if_defined =
            [node]<typename TReturn>(const std::string& value,
                                     const TReturn& default_value) -> TReturn {
            if (!node[value].IsDefined()) { return default_value; }
            return node[value].as<TReturn>();
        };

        result.repo_url = get_if_defined("repo-url", result.repo_url);
        result.repo_structure_template = get_if_defined(
            "repo-structure-template", result.repo_structure_template);
        result.download_path = get_if_defined.operator()<std::string>(
            "pool-path", result.download_path);
        const auto exclude_list_path = get_if_defined.operator()<std::string>(
            "exclude-list-path", "exclude_list");
        std::ifstream file(exclude_list_path);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                result.exclude_list.insert(std::move(line));
            }
            file.close();
        }

        return result;
    };

    std::string repo_url = "cloudflaremirrors.com";
    std::string repo_structure_template =
        "/archlinux/{repository}/os/{architecture}";
    std::filesystem::path download_path = "/var/tmp/bxt/";
    phmap::parallel_flat_hash_set<std::string> exclude_list;

    std::optional<std::string> repo_name;
};

} // namespace bxt::Infrastructure
