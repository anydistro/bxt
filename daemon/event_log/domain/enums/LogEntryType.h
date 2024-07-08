/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

namespace bxt::EventLog::Domain {

enum class LogEntryType { Add, Remove, Update };

enum class EventLogEntryType { Commit, Deploy, Sync };

} // namespace bxt::EventLog::Domain
