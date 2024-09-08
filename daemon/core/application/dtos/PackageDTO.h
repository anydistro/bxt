/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/entities/Package.h"
#include "core/domain/value_objects/PackagePoolEntry.h"
#include "core/domain/value_objects/PackageVersion.h"
#include "PackageSectionDTO.h"
#include "parallel_hashmap/phmap.h"
#include "utilities/log/Logging.h"
#include "utilities/StaticDTOMapper.h"

#include <cereal/types/optional.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <utilities/CerealFsPath.h>

namespace bxt::Core::Application {

struct PackagePoolEntryDTO {
    std::string version;

    std::filesystem::path filepath;
    std::optional<std::filesystem::path> signature_path = {};

    auto operator<=>(PackagePoolEntryDTO const& other) const = default;

    template<class Archive> void serialize(Archive& ar) {
        ar(version, filepath, signature_path);
    }
};

struct PackageDTO {
    PackageSectionDTO section;
    std::string name;
    bool is_any_architecture = false;
    phmap::flat_hash_map<Domain::PoolLocation, PackagePoolEntryDTO> pool_entries;

    auto operator<=>(PackageDTO const& other) const = default;

    template<class Archive> void serialize(Archive& ar) {
        ar(section, name, pool_entries);
    }
};

using PackageDTOMapper = bxt::Utilities::StaticDTOMapper<bxt::Core::Domain::Package, PackageDTO>;

} // namespace bxt::Core::Application

namespace {
using namespace bxt::Core::Domain;
using namespace bxt::Core::Application;
} // namespace

template<> struct bxt::Utilities::StaticDTOMapper<Package, PackageDTO> {
    static PackageDTO to_dto(Package const& from) {
        PackageDTO dto;
        dto.section = SectionDTOMapper::to_dto(from.section());
        dto.name = from.name();

        for (auto const& [location, entry] : from.pool_entries()) {
            dto.pool_entries.emplace(location, PackagePoolEntryDTO {entry.version().string(),
                                                                    entry.file_path(),
                                                                    entry.signature_path()});
        }
        return dto;
    }

    static Package to_entity(PackageDTO const& from) {
        Package package(
            SectionDTOMapper::to_entity(from.section),
            from.name.empty()
                ? *Package::parse_file_name(from.pool_entries.begin()->second.filepath.filename())
                : from.name,
            from.is_any_architecture);

        for (auto const& entry : from.pool_entries) {
            auto entity = PackagePoolEntry::parse_file_path(entry.second.filepath,
                                                            entry.second.signature_path);

            if (!entity.has_value()) {
                loge("Failed to parse package pool entry: {}", entry.second.filepath.string());
                continue;
            }
            package.pool_entries().emplace(entry.first, *entity);
        }
        return package;
    }
};
