/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/entities/Package.h"
#include "core/domain/value_objects/PackageVersion.h"
#include "persistence/box/record/PackageRecord.h"

namespace bxt::Persistence::Box::RecordMapper {

static PackageRecord to_record(const Core::Domain::Package &from) {
    PackageRecord result {
        .id = {.section = SectionDTOMapper::to_dto(from.section()),
               .name = from.name()},
        .is_any_architecture = from.is_any_arch()};

    for (const auto &[location, entry] : from.pool_entries()) {
        result.descriptions[location].filepath = entry.file_path();
        result.descriptions[location].signature_path = entry.signature_path();
        result.descriptions[location].descfile = entry.desc();
    }

    return result;
}
static Core::Domain::Package to_entity(const PackageRecord &from) {
    Core::Domain::Package result(SectionDTOMapper::to_entity(from.id.section),
                                 from.id.name, from.is_any_architecture);

    for (const auto &[location, entry] : from.descriptions) {
        auto version_str = entry.descfile.get("VERSION");
        if (!version_str) { continue; }
        auto version_result = PackageVersion::from_string(*version_str);

        if (!version_result) { continue; }

        Core::Domain::PackagePoolEntry pool_entry(
            entry.filepath, entry.signature_path, entry.descfile,
            *version_result);

        result.pool_entries().emplace(location, pool_entry);
    }

    return result;
}
}; // namespace bxt::Persistence::Box::RecordMapper
