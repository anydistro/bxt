/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "event_log/domain/entities/EventLogEntry.h"
#include "event_log/domain/entities/PackageLogEntry.h"

#include <string>
#include <utility>
#include <vector>

namespace bxt::EventLog::Domain {

class DeployLogEntry : public EventLogEntryBase {
public:
    DeployLogEntry(const time_point& time,
                   std::string runner_url,
                   const std::vector<PackageLogEntry>& added)
        : EventLogEntryBase(EventLogEntryType::Deploy, time),
          m_runner_url(std::move(runner_url)),
          m_added(added) {}

    std::string runner_url() const { return m_runner_url; }

    std::vector<PackageLogEntry> added() const { return m_added; }

private:
    std::string m_runner_url;
    std::vector<PackageLogEntry> m_added;
};

} // namespace bxt::EventLog::Domain
