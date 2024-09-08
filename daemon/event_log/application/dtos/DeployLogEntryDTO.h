/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "event_log/application/dtos/EventLogEntryDTO.h"
#include "event_log/application/dtos/PackageLogEntryDTO.h"
#include "event_log/domain/entities/DeployLogEntry.h"
#include "utilities/StaticDTOMapper.h"

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <string>

namespace bxt::EventLog::Application {
struct DeployLogEntryDTO : public EventLogEntryDTO {
    std::string runner_url;
    std::vector<PackageLogEntryDTO> added;

    template<class Archive> void serialize(Archive& ar) {
        EventLogEntryDTO::serialize(ar);
        ar(runner_url, added);
    }
};

using DeployLogEntryDTOMapper =
    bxt::Utilities::StaticDTOMapper<Domain::DeployLogEntry, DeployLogEntryDTO>;
} // namespace bxt::EventLog::Application

namespace bxt::Utilities {

using bxt::EventLog::Application::DeployLogEntryDTO;
using bxt::EventLog::Domain::DeployLogEntry;
using EventLog::Application::PackageLogEntryDTOMapper;

template<> struct StaticDTOMapper<DeployLogEntry, DeployLogEntryDTO> {
    static DeployLogEntryDTO to_dto(DeployLogEntry const& from) {
        DeployLogEntryDTO dto;
        dto.time = from.time();

        dto.runner_url = from.runner_url();
        dto.added = map_entries(from.added(), PackageLogEntryDTOMapper::to_dto);

        return dto;
    }

    static DeployLogEntry to_entity(DeployLogEntryDTO const& from) {
        return DeployLogEntry {from.time, from.runner_url,
                               map_entries(from.added, PackageLogEntryDTOMapper::to_entity)};
    }
};
} // namespace bxt::Utilities
