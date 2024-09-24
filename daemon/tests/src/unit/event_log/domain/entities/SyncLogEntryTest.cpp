
/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "event_log/domain/entities/SyncLogEntry.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::EventLog::Domain;
using namespace bxt::Core::Domain;

// Helper functions to create PackageLogEntry objects
PackageLogEntry create_pkg_entry1() {
    Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
    return PackageLogEntry {LogEntryType::Add, section, "pkg1", PoolLocation::Sync,
                            PackageVersion::from_string("1.0").value()};
}

PackageLogEntry create_pkg_entry2() {
    Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
    return PackageLogEntry {LogEntryType::Add, section, "pkg2", PoolLocation::Sync,
                            PackageVersion::from_string("2.0").value()};
}

PackageLogEntry create_pkg_entry3() {
    Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
    return PackageLogEntry {LogEntryType::Remove, section, "pkg3", PoolLocation::Sync,
                            PackageVersion::from_string("3.0").value()};
}

TEST_CASE("SyncLogEntry entity", "[event_log][domain][entities]") {
    SECTION("Construction and getters") {
        std::vector<PackageLogEntry> added = {create_pkg_entry1(), create_pkg_entry2()};
        std::vector<PackageLogEntry> deleted = {create_pkg_entry3()};

        SyncLogEntry entry(std::chrono::system_clock::now(), "John Doe", added, deleted);

        REQUIRE(entry.sync_trigger_username() == "John Doe");
        REQUIRE(entry.added() == added);
        REQUIRE(entry.deleted() == deleted);
        REQUIRE(entry.type() == EventLogEntryType::Sync);
    }

    SECTION("Time comparison") {
        auto time1 = std::chrono::system_clock::now();
        auto time2 = time1 + std::chrono::hours(1);

        SyncLogEntry entry1(time1, "Alice", {}, {});
        SyncLogEntry entry2(time2, "Bob", {}, {});

        REQUIRE(entry1.time() < entry2.time());
    }

    SECTION("Empty collections") {
        SyncLogEntry entry(std::chrono::system_clock::now(), "Empty", {}, {});

        REQUIRE(entry.added().empty());
        REQUIRE(entry.deleted().empty());
    }
}
