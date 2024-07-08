/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "event_log/domain/enums/LogEntryType.h"
#include "presentation/messages/PackageMessages.h"

#include <chrono>
#include <string>

namespace bxt::Presentation {

struct PackageLogEntryResponse {
    EventLog::Domain::LogEntryType type;
    PackageSectionDTO section;
    std::string name;
    std::string location;
    std::optional<std::string> version;
};
struct CommitLogEntryResponse {
    std::chrono::system_clock::time_point time;
    std::string commiter_username;
    std::vector<PackageLogEntryResponse> added;
    std::vector<PackageLogEntryResponse> deleted;
    std::vector<std::pair<PackageLogEntryResponse, PackageLogEntryResponse>>
        moved;
    std::vector<std::pair<PackageLogEntryResponse, PackageLogEntryResponse>>
        copied;
};

struct SyncLogEntryResponse {
    std::chrono::system_clock::time_point time;
    std::string sync_trigger_username;
    std::vector<PackageLogEntryResponse> added;
    std::vector<PackageLogEntryResponse> deleted;
};

struct DeployLogEntryResponse {
    std::chrono::system_clock::time_point time;
    std::string runner_url;
    std::vector<PackageLogEntryResponse> added;
};

struct LogResponse {
    std::vector<CommitLogEntryResponse> commits;
    std::vector<SyncLogEntryResponse> syncs;
    std::vector<DeployLogEntryResponse> deploys;
};

} // namespace bxt::Presentation
