/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "event_log/domain/entities/DeployLogEntry.h"

#include "helpers.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::EventLog::Domain;
using namespace bxt::Core::Domain;

TEST_CASE("DeployLogEntry entity", "[event_log][domain][entities]") {
    SECTION("Construction and getters") {
        Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
        std::vector<PackageLogEntry> added = {bxt::tests::create_pkg_entry1(),
                                              bxt::tests::create_pkg_entry2()};

        DeployLogEntry entry(std::chrono::system_clock::now(),
                             "https://example.com/actions/run/12345", added);

        REQUIRE(entry.runner_url() == "https://example.com/actions/run/12345");
        REQUIRE(entry.added() == added);

        REQUIRE(entry.type() == EventLogEntryType::Deploy);
    }

    SECTION("Time comparison") {
        auto time1 = std::chrono::system_clock::now();
        auto time2 = time1 + std::chrono::hours(1);

        DeployLogEntry entry1(time1, "https://example.com/actions/run/12345", {});
        DeployLogEntry entry2(time2, "https://example.com/actions/run/54321", {});

        REQUIRE(entry1.time() < entry2.time());
    }

    SECTION("Empty collections") {
        DeployLogEntry entry(std::chrono::system_clock::now(),
                             "https://example.com/actions/run/00000", {});

        REQUIRE(entry.added().empty());
    }
}
