/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <cereal/types/chrono.hpp>
#include <chrono>

namespace bxt::EventLog::Application {

struct EventLogEntryDTO {
    using time_point = std::chrono::system_clock::time_point;

    time_point time;
    template<class Archive> void serialize(Archive& ar) { ar(time); }
};

} // namespace bxt::EventLog::Application
