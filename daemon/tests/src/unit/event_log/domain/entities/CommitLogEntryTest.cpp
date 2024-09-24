/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "event_log/domain/entities/CommitLogEntry.h"

#include "helpers.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::EventLog::Domain;
using namespace bxt::Core::Domain;

// Helper functions to create PackageLogEntry objects

TEST_CASE("CommitLogEntry entity", "[event_log][domain][entities]") {
    SECTION("Construction and getters") {
        Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
        std::vector<PackageLogEntry> added = {bxt::tests::create_pkg_entry1(),
                                              bxt::tests::create_pkg_entry2()};
        std::vector<PackageLogEntry> deleted = {bxt::tests::create_pkg_entry3()};
        std::vector<PackageUpdateLogEntry> moved = {PackageUpdateLogEntry(
            bxt::tests::create_pkg_entry4(), bxt::tests::create_pkg_entry4())};
        std::vector<PackageUpdateLogEntry> copied = {PackageUpdateLogEntry(
            bxt::tests::create_pkg_entry5(), bxt::tests::create_pkg_entry5())};

        CommitLogEntry entry(std::chrono::system_clock::now(), "John Doe", added, deleted, moved,
                             copied);

        REQUIRE(entry.commiter_name() == "John Doe");
        REQUIRE(entry.added() == added);
        REQUIRE(entry.deleted() == deleted);
        REQUIRE(entry.moved() == moved);
        REQUIRE(entry.copied() == copied);
        REQUIRE(entry.type() == EventLogEntryType::Commit);
    }

    SECTION("Time comparison") {
        auto time1 = std::chrono::system_clock::now();
        auto time2 = time1 + std::chrono::hours(1);

        CommitLogEntry entry1(time1, "Alice", {}, {}, {}, {});
        CommitLogEntry entry2(time2, "Bob", {}, {}, {}, {});

        REQUIRE(entry1.time() < entry2.time());
    }

    SECTION("Empty collections") {
        CommitLogEntry entry(std::chrono::system_clock::now(), "Empty", {}, {}, {}, {});

        REQUIRE(entry.added().empty());
        REQUIRE(entry.deleted().empty());
        REQUIRE(entry.moved().empty());
        REQUIRE(entry.copied().empty());
    }
}
