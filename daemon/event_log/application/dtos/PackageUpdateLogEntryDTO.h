/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "event_log/application/dtos/PackageLogEntryDTO.h"
#include "event_log/domain/entities/PackageUpdateLogEntry.h"
#include "utilities/StaticDTOMapper.h"

namespace bxt::EventLog::Application {
struct PackageUpdateLogEntryDTO {
    PackageLogEntryDTO package;
    PackageLogEntryDTO previous_package;

    template<class Archive> void serialize(Archive &ar) {
        ar(package, previous_package);
    }
};

using PackageUpdateLogEntryDTOMapper =
    bxt::Utilities::StaticDTOMapper<Domain::PackageUpdateLogEntry,
                                    PackageUpdateLogEntryDTO>;

} // namespace bxt::EventLog::Application

namespace bxt::Utilities {

using PackageUpdateLogEntryDTO =
    bxt::EventLog::Application::PackageUpdateLogEntryDTO;
using PackageLogEntryDTOMapper =
    bxt::EventLog::Application::PackageLogEntryDTOMapper;
using PackageUpdateLogEntry = bxt::EventLog::Domain::PackageUpdateLogEntry;

template<>
struct StaticDTOMapper<PackageUpdateLogEntry, PackageUpdateLogEntryDTO> {
    static PackageUpdateLogEntryDTO to_dto(const PackageUpdateLogEntry &from) {
        PackageUpdateLogEntryDTO dto;
        dto.package = PackageLogEntryDTOMapper::to_dto(from.package());
        dto.previous_package =
            PackageLogEntryDTOMapper::to_dto(from.previous_package());
        return dto;
    }

    static PackageUpdateLogEntry
        to_entity(const PackageUpdateLogEntryDTO &from) {
        PackageUpdateLogEntry entity(
            PackageLogEntryDTOMapper::to_entity(from.package),
            PackageLogEntryDTOMapper::to_entity(from.previous_package));
        return entity;
    }
};
} // namespace bxt::Utilities
