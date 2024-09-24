/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/value_objects/PackageVersion.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::Core::Domain;

TEST_CASE("PackageVersion", "[core][domain][value_objects]") {
    SECTION("Parse valid version string") {
        auto result = PackageVersion::from_string("1:2.3.4-5");
        REQUIRE(result.has_value());
        auto version = result.value();
        REQUIRE(version.epoch == Name("1"));
        REQUIRE(version.version == Name("2.3.4"));
        REQUIRE(version.release.has_value());
        REQUIRE(version.release.value() == Name("5"));
    }

    SECTION("Parse version string without epoch") {
        auto result = PackageVersion::from_string("2.3.4-5");
        REQUIRE(result.has_value());
        auto version = result.value();
        REQUIRE(version.epoch == Name("0"));
        REQUIRE(version.version == Name("2.3.4"));
        REQUIRE(version.release.has_value());
        REQUIRE(version.release.value() == Name("5"));
    }

    SECTION("Parse version string without release") {
        auto result = PackageVersion::from_string("1:2.3.4");
        REQUIRE(result.has_value());
        auto version = result.value();
        REQUIRE(version.epoch == Name("1"));
        REQUIRE(version.version == Name("2.3.4"));
        REQUIRE_FALSE(version.release.has_value());
    }

    SECTION("Compare versions") {
        auto v1 = PackageVersion::from_string("1:2.3.4-5").value();
        auto v2 = PackageVersion::from_string("1:2.3.4-6").value();
        auto v3 = PackageVersion::from_string("2:1.0.0-1").value();

        REQUIRE(v1 < v2);
        REQUIRE(v2 < v3);
        REQUIRE(v1 < v3);
    }

    SECTION("Version to string") {
        auto version = PackageVersion::from_string("1:2.3.4-5").value();
        REQUIRE(version.string() == "1:2.3.4-5");

        auto version_no_epoch = PackageVersion::from_string("2.3.4-5").value();
        REQUIRE(version_no_epoch.string() == "2.3.4-5");

        auto version_no_release = PackageVersion::from_string("1:2.3.4").value();
        REQUIRE(version_no_release.string() == "1:2.3.4");

        auto version_simple = PackageVersion::from_string("2.3.4").value();
        REQUIRE(version_simple.string() == "2.3.4");
    }
}
