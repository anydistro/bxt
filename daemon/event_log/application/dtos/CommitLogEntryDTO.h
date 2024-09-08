/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "event_log/application/dtos/EventLogEntryDTO.h"
#include "event_log/application/dtos/PackageLogEntryDTO.h"
#include "event_log/application/dtos/PackageUpdateLogEntryDTO.h"
#include "event_log/domain/entities/CommitLogEntry.h"
#include "utilities/StaticDTOMapper.h"

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <string>
#include <vector>

namespace bxt::EventLog::Application {
struct CommitLogEntryDTO : public EventLogEntryDTO {
    std::string commiter_username;
    std::vector<PackageLogEntryDTO> added;
    std::vector<PackageLogEntryDTO> deleted;
    std::vector<PackageUpdateLogEntryDTO> moved;
    std::vector<PackageUpdateLogEntryDTO> copied;

    template<class Archive> void serialize(Archive& ar) {
        EventLogEntryDTO::serialize(ar);
        ar(commiter_username, added, deleted, moved, copied);
    }
};

using CommitLogEntryDTOMapper =
    bxt::Utilities::StaticDTOMapper<Domain::CommitLogEntry, CommitLogEntryDTO>;
} // namespace bxt::EventLog::Application

namespace bxt::Utilities {

using bxt::EventLog::Application::CommitLogEntryDTO;
using bxt::EventLog::Domain::CommitLogEntry;
using EventLog::Application::PackageLogEntryDTOMapper;
using EventLog::Application::PackageUpdateLogEntryDTOMapper;

template<> struct StaticDTOMapper<CommitLogEntry, CommitLogEntryDTO> {
    static CommitLogEntryDTO to_dto(CommitLogEntry const& from) {
        CommitLogEntryDTO dto;
        dto.time = from.time();
        dto.commiter_username = from.commiter_name();
        dto.added = map_entries(from.added(), PackageLogEntryDTOMapper::to_dto);
        dto.deleted = map_entries(from.deleted(), PackageLogEntryDTOMapper::to_dto);
        dto.moved = map_entries(from.moved(), PackageUpdateLogEntryDTOMapper::to_dto);
        dto.copied = map_entries(from.copied(), PackageUpdateLogEntryDTOMapper::to_dto);
        return dto;
    }

    static CommitLogEntry to_entity(CommitLogEntryDTO const& from) {
        return CommitLogEntry {from.time,
                               from.commiter_username,
                               map_entries(from.added, PackageLogEntryDTOMapper::to_entity),
                               map_entries(from.deleted, PackageLogEntryDTOMapper::to_entity),
                               map_entries(from.moved, PackageUpdateLogEntryDTOMapper::to_entity),
                               map_entries(from.copied, PackageUpdateLogEntryDTOMapper::to_entity)};
    }
};
} // namespace bxt::Utilities
