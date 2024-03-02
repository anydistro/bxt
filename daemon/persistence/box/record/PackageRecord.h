/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/enums/PoolLocation.h"
#include "utilities/alpmdb/Desc.h"

#include <optional>
#include <string>
#include <string_view>

#define USE_CEREAL 1

#include <cereal/archives/binary.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <filesystem>
#include <parallel_hashmap/phmap.h>
#include <utilities/CerealFsPath.h>

namespace bxt::Persistence::Box {

struct PackageRecord {
    struct Id {
        Core::Application::PackageSectionDTO section;
        std::string name;

        std::string to_string() const {
            return fmt::format("{}/{}", std::string(section), name);
        }

        static std::optional<Id> from_string(const std::string_view key) {
            std::vector<std::string> parts;
            boost::split(parts, key, boost::is_any_of("/"));
            if (parts.size() != 4) { return {}; }

            return Id {PackageSectionDTO {.branch = parts[0],
                                          .repository = parts[1],
                                          .architecture = parts[2]},
                       parts[3]};
        }

        template<typename Archive> void serialize(Archive& ar) {
            ar(section, name);
        }
    };
    struct Description {
        std::filesystem::path filepath;
        std::optional<std::filesystem::path> signature_path = {};
        Utilities::AlpmDb::Desc descfile;

        template<typename Archive> void serialize(Archive& ar) {
            ar(filepath, signature_path, descfile);
        }
    };

    Id id;

    bool is_any_architecture = false;
    phmap::flat_hash_map<Core::Domain::PoolLocation, Description> descriptions;

    template<typename Archive> void serialize(Archive& ar) {
        ar(id, is_any_architecture, descriptions);
    }
};
} // namespace bxt::Persistence::Box
