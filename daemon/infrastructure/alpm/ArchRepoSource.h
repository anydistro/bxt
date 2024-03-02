/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/repo-schema/SchemaExtension.h"

#include <filesystem>
#include <optional>
#include <string>

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

        return result;
    };

    std::string repo_url = "cloudflaremirrors.com";
    std::string repo_structure_template =
        "/archlinux/{repository}/os/{architecture}";
    std::filesystem::path download_path = "/var/tmp/bxt/";

    std::optional<std::string> repo_name;
};

} // namespace bxt::Infrastructure
