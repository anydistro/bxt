/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "event_log/domain/entities/EventLogEntry.h"
#include "event_log/domain/entities/PackageLogEntry.h"
#include "event_log/domain/entities/PackageUpdateLogEntry.h"

#include <vector>

namespace bxt::EventLog::Domain {
class CommitLogEntry : public EventLogEntryBase {
public:
    CommitLogEntry(time_point const& time,
                   std::string commiter_name,
                   std::vector<PackageLogEntry> const& added,
                   std::vector<PackageLogEntry> const& deleted,
                   std::vector<PackageUpdateLogEntry> const& moved,
                   std::vector<PackageUpdateLogEntry> const& copied)
        : EventLogEntryBase(EventLogEntryType::Commit, time)
        , m_commiter_name(std::move(commiter_name))
        , m_added(added)
        , m_deleted(deleted)
        , m_moved(moved)
        , m_copied(copied) {
    }

    std::string const& commiter_name() const {
        return m_commiter_name;
    }
    std::vector<PackageLogEntry> const& added() const {
        return m_added;
    }
    std::vector<PackageLogEntry> const& deleted() const {
        return m_deleted;
    }
    std::vector<PackageUpdateLogEntry> const& moved() const {
        return m_moved;
    }
    std::vector<PackageUpdateLogEntry> const& copied() const {
        return m_copied;
    }

private:
    std::string m_commiter_name;
    std::vector<PackageLogEntry> m_added;
    std::vector<PackageLogEntry> m_deleted;
    std::vector<PackageUpdateLogEntry> m_moved;
    std::vector<PackageUpdateLogEntry> m_copied;
};

} // namespace bxt::EventLog::Domain
