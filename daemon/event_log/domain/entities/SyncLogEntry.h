/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "event_log/domain/entities/EventLogEntry.h"
#include "event_log/domain/entities/PackageLogEntry.h"

#include <utility>
#include <vector>

namespace bxt::EventLog::Domain {

class SyncLogEntry : public EventLogEntryBase {
public:
    SyncLogEntry(const time_point& time,
                 std::string username,
                 const std::vector<PackageLogEntry>& added,
                 const std::vector<PackageLogEntry>& deleted)
        : EventLogEntryBase(EventLogEntryType::Sync, time),
          m_sync_trigger_username(std::move(username)),
          m_added {added},
          m_deleted {deleted} {}

    std::string sync_trigger_username() const {
        return m_sync_trigger_username;
    }

    std::vector<PackageLogEntry> added() const { return m_added; }

    std::vector<PackageLogEntry> deleted() const { return m_deleted; }

private:
    std::string m_sync_trigger_username;
    std::vector<PackageLogEntry> m_added;
    std::vector<PackageLogEntry> m_deleted;
};
} // namespace bxt::EventLog::Domain
