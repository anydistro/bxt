/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "event_log/domain/entities/PackageLogEntry.h"
#include "event_log/domain/enums/LogEntryType.h"
#include "utilities/StaticDTOMapper.h"

#include <cereal/types/chrono.hpp>
#include <optional>
#include <string>

namespace bxt::EventLog::Application {

struct PackageLogEntryDTO {
    Domain::LogEntryType type = Domain::LogEntryType::Add;
    PackageSectionDTO section;
    std::string name;
    PoolLocation location = PoolLocation::Unknown;
    std::optional<std::string> version;

    template<class Archive> void serialize(Archive& ar) {
        ar(type, section, name, location, version);
    }
};

using PackageLogEntryDTOMapper =
    bxt::Utilities::StaticDTOMapper<Domain::PackageLogEntry, PackageLogEntryDTO>;

} // namespace bxt::EventLog::Application
namespace bxt::Utilities {

template<>
struct bxt::Utilities::StaticDTOMapper<bxt::EventLog::Domain::PackageLogEntry,
                                       bxt::EventLog::Application::PackageLogEntryDTO> {
    static EventLog::Application::PackageLogEntryDTO
        to_dto(EventLog::Domain::PackageLogEntry const& from) {
        EventLog::Application::PackageLogEntryDTO dto;

        dto.type = from.type();
        dto.section = SectionDTOMapper::to_dto(from.section());
        dto.name = from.name();
        dto.location = from.location();
        if (from.version().has_value()) {
            dto.version = bxt::to_string(from.version().value());
        }

        return dto;
    }

    static EventLog::Domain::PackageLogEntry
        to_entity(EventLog::Application::PackageLogEntryDTO const& from) {
        EventLog::Domain::PackageLogEntry entity(
            from.type, SectionDTOMapper::to_entity(from.section), from.name, from.location,
            from.version.has_value()
                ? std::make_optional(*PackageVersion::from_string(from.version.value()))
                : std::nullopt);
        return entity;
    }
};
} // namespace bxt::Utilities
