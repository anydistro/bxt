
/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "event_log/domain/entities/PackageUpdateLogEntry.h"

#include "core/domain/entities/Section.h"
#include "core/domain/enums/PoolLocation.h"
#include "core/domain/value_objects/PackageVersion.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::Core::Domain;
using namespace bxt::EventLog::Domain;

TEST_CASE("PackageUpdateLogEntry", "[event_log][domain][entities]") {
    SECTION("Construction and basic properties") {
        Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
        PackageLogEntry old_entry {LogEntryType::Add, section, "test-package", PoolLocation::Sync,
                                   PackageVersion::from_string("1.0.0-1").value()};
        PackageLogEntry new_entry {LogEntryType::Update, section, "test-package",
                                   PoolLocation::Sync,
                                   PackageVersion::from_string("1.1.0-1").value()};

        PackageUpdateLogEntry update_entry {new_entry, old_entry};

        REQUIRE(update_entry.package().name() == "test-package");
        REQUIRE(update_entry.package().version()->string() == "1.1.0-1");
        REQUIRE(update_entry.package().section().string() == "stable/core/x86_64");
        REQUIRE(update_entry.package().location() == PoolLocation::Sync);

        REQUIRE(update_entry.previous_package().name() == "test-package");
        REQUIRE(update_entry.previous_package().version()->string() == "1.0.0-1");
        REQUIRE(update_entry.previous_package().section().string() == "stable/core/x86_64");
        REQUIRE(update_entry.previous_package().location() == PoolLocation::Sync);
    }

    SECTION("Equality comparison") {
        Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
        PackageLogEntry old_entry {LogEntryType::Add, section, "test-package", PoolLocation::Sync,
                                   PackageVersion::from_string("1.0.0-1").value()};
        PackageLogEntry new_entry {LogEntryType::Update, section, "test-package",
                                   PoolLocation::Sync,
                                   PackageVersion::from_string("1.1.0-1").value()};

        PackageUpdateLogEntry update_entry1 {new_entry, old_entry};
        PackageUpdateLogEntry update_entry2 {new_entry, old_entry};

        REQUIRE(update_entry1 == update_entry2);
    }
}
