/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "event_log/domain/entities/PackageLogEntry.h"

#include "core/domain/entities/Section.h"
#include "core/domain/enums/PoolLocation.h"
#include "core/domain/value_objects/PackageVersion.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::Core::Domain;
using namespace bxt::EventLog::Domain;

TEST_CASE("PackageLogEntry", "[event_log][domain][entities]") {
    SECTION("Construction and basic properties") {
        Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
        PackageLogEntry entry {LogEntryType::Add, section, "test-package", PoolLocation::Sync,
                               PackageVersion::from_string("1.0.0-1").value()};

        REQUIRE(entry.name() == "test-package");
        REQUIRE(entry.version()->string() == "1.0.0-1");
        REQUIRE(entry.section().string() == "stable/core/x86_64");
        REQUIRE(entry.location() == PoolLocation::Sync);
    }
}
