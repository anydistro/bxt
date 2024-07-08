/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "event_log/application/dtos/CommitLogEntryDTO.h"
#include "event_log/application/dtos/DeployLogEntryDTO.h"
#include "event_log/application/dtos/SyncLogEntryDTO.h"
#include "event_log/domain/entities/CommitLogEntry.h"
#include "event_log/domain/entities/SyncLogEntry.h"
#include "persistence/lmdb/LmdbRepository.h"

namespace bxt::Persistence::LMDB {
using SyncLogEntryRepository =
    LmdbRepositoryBase<EventLog::Domain::SyncLogEntry,
                       EventLog::Application::SyncLogEntryDTO>;

using CommitLogEntryRepository =
    LmdbRepositoryBase<EventLog::Domain::CommitLogEntry,
                       EventLog::Application::CommitLogEntryDTO>;

using DeployLogEntryRepository =
    LmdbRepositoryBase<EventLog::Domain::DeployLogEntry,
                       EventLog::Application::DeployLogEntryDTO>;
} // namespace bxt::Persistence::LMDB
