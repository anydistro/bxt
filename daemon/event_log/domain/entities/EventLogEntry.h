/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "event_log/domain/enums/LogEntryType.h"

#include <chrono>

namespace bxt::EventLog::Domain {

using time_point = std::chrono::system_clock::time_point;

class EventLogEntryBase {
public:
    virtual ~EventLogEntryBase() = default;
    using TId = std::string;

    EventLogEntryBase(EventLogEntryType type, const time_point& time)
        : m_time {time}, m_type {type} {}

    std::string id() const {
        return std::to_string(m_time.time_since_epoch().count());
    }
    time_point time() const { return m_time; }

    EventLogEntryType type() const { return m_type; }

private:
    time_point m_time;
    EventLogEntryType m_type;
};

} // namespace bxt::EventLog::Domain
