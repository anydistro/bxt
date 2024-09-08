/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "event_log/application/dtos/EventLogEntryDTO.h"
#include "event_log/application/dtos/PackageLogEntryDTO.h"
#include "event_log/domain/entities/SyncLogEntry.h"
#include "utilities/StaticDTOMapper.h"

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <string>

namespace bxt::EventLog::Application {
struct SyncLogEntryDTO : public EventLogEntryDTO {
    std::string sync_trigger_username;
    std::vector<PackageLogEntryDTO> added;
    std::vector<PackageLogEntryDTO> deleted;

    template<class Archive> void serialize(Archive& ar) {
        EventLogEntryDTO::serialize(ar);
        ar(sync_trigger_username, added, deleted);
    }
};

using SyncLogEntryDTOMapper =
    bxt::Utilities::StaticDTOMapper<Domain::SyncLogEntry, SyncLogEntryDTO>;
} // namespace bxt::EventLog::Application

namespace bxt::Utilities {

using bxt::EventLog::Application::SyncLogEntryDTO;
using bxt::EventLog::Domain::SyncLogEntry;
using EventLog::Application::PackageLogEntryDTOMapper;

template<> struct StaticDTOMapper<SyncLogEntry, SyncLogEntryDTO> {
    static SyncLogEntryDTO to_dto(SyncLogEntry const& from) {
        SyncLogEntryDTO dto;
        dto.time = from.time();

        dto.sync_trigger_username = from.sync_trigger_username();
        dto.added = map_entries(from.added(), PackageLogEntryDTOMapper::to_dto);
        dto.deleted = map_entries(from.deleted(), PackageLogEntryDTOMapper::to_dto);

        return dto;
    }

    static SyncLogEntry to_entity(SyncLogEntryDTO const& from) {
        return SyncLogEntry {from.time, from.sync_trigger_username,
                             map_entries(from.added, PackageLogEntryDTOMapper::to_entity),
                             map_entries(from.deleted, PackageLogEntryDTOMapper::to_entity)};
    }
};
} // namespace bxt::Utilities
