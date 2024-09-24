
/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "event_log/domain/entities/EventLogEntry.h"

#include <catch2/catch_test_macros.hpp>
#include <chrono>

using namespace bxt::EventLog::Domain;

TEST_CASE("EventLogEntryBase", "[event_log][domain][entities]") {
    SECTION("Construction and basic properties") {
        auto now = std::chrono::system_clock::now();
        EventLogEntryBase entry {EventLogEntryType::Commit, now};

        REQUIRE(entry.id() == std::to_string(now.time_since_epoch().count()));
        REQUIRE(entry.time() == now);
        REQUIRE(entry.type() == EventLogEntryType::Commit);
    }

    SECTION("Different types") {
        auto now = std::chrono::system_clock::now();
        EventLogEntryBase commit_entry {EventLogEntryType::Commit, now};
        EventLogEntryBase deploy_entry {EventLogEntryType::Deploy, now};

        REQUIRE(commit_entry.type() == EventLogEntryType::Commit);
        REQUIRE(deploy_entry.type() == EventLogEntryType::Deploy);
    }

    SECTION("Unique IDs for different time points") {
        auto time1 = std::chrono::system_clock::now();
        auto time2 = time1 + std::chrono::seconds(1);

        EventLogEntryBase entry1 {EventLogEntryType::Commit, time1};
        EventLogEntryBase entry2 {EventLogEntryType::Deploy, time2};

        REQUIRE(entry1.id() != entry2.id());
    }
}
