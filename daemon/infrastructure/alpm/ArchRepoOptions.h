/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "infrastructure/alpm/ArchRepoSource.h"
#include "utilities/repo-schema/SchemaExtension.h"

#include <filesystem>
#include <parallel_hashmap/phmap.h>
#include <yaml-cpp/yaml.h>

namespace bxt::Infrastructure {

struct ArchRepoOptions : public Utilities::RepoSchema::Extension {
    phmap::parallel_flat_hash_map<Core::Application::PackageSectionDTO,
                                  ArchRepoSource>
        sources;
    std::filesystem::path download_path = "/var/cache/bxt/packages";

    virtual void parse(const YAML::Node& root_node) override {
        constexpr char Tag[] = "(alpm.sync)";

        const auto& options_node = root_node[Tag];

        if (options_node["download-path"].IsDefined()
            && options_node["download-path"].IsScalar()) {
            download_path = options_node["download-path"].as<std::string>();
        }
        for (const auto& branch :
             options_node["sync-branches"].as<std::vector<std::string>>()) {
            for (const auto& repo : root_node["repositories"]) {
                const auto& key = repo.first;
                const auto& value = repo.second;
                if (!key || !value || !value.IsMap()) { continue; }

                auto source_options = value[Tag];

                auto architecture = value["architecture"].as<std::string>();

                auto source = ArchRepoSource::from_node(source_options);

                std::vector<std::string> source_repo_names;

                if (source_options["repo-names"].IsSequence()) {
                    source_repo_names = source_options["repo-names"]
                                            .as<std::vector<std::string>>();
                }

                if (key.IsScalar()) {
                    if (!source_repo_names.empty()) {
                        source.repo_name = source_repo_names[0];
                    }

                    sources.emplace(PackageSectionDTO {branch,
                                                       key.as<std::string>(),
                                                       architecture},
                                    std::move(source));
                } else if (key.IsSequence()) {
                    auto repos = key.as<std::vector<std::string>>();

                    for (std::size_t i = 0; i < repos.size(); i++) {
                        auto named_source = source;
                        if (source_repo_names.size() > i) {
                            named_source.repo_name = source_repo_names[i];
                        }

                        sources.emplace(
                            PackageSectionDTO {branch, repos[i], architecture},
                            named_source);
                    }
                }
            }
        }
    }
};

} // namespace bxt::Infrastructure
