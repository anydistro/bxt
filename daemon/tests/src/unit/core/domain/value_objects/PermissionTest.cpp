/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/value_objects/Permission.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::Core::Domain;

TEST_CASE("Permission", "[core][domain][value_objects]") {
    SECTION("Create permission from string") {
        Permission permission("sections.stable.core.x86_64");
        REQUIRE(permission.tags().size() == 4);
        REQUIRE(permission.tags()[0] == "sections");
        REQUIRE(permission.tags()[1] == "stable");
        REQUIRE(permission.tags()[2] == "core");
        REQUIRE(permission.tags()[3] == "x86_64");
    }

    SECTION("Convert permission to string") {
        Permission permission("packages.get.unstable.extra.arm");
        std::string permission_str = permission;
        REQUIRE(permission_str == "packages.get.unstable.extra.arm");
    }

    SECTION("Compare permissions") {
        Permission p1("users.add");
        Permission p2("users.remove");
        Permission p3("users.add");

        REQUIRE(p1 < p2);
        REQUIRE(p1 == p3);
        REQUIRE(p2 > p1);
    }

    SECTION("Create permission with single tag") {
        Permission permission("logs");
        REQUIRE(permission.tags().size() == 1);
        REQUIRE(permission.tags()[0] == "logs");
    }

    SECTION("Create permission with multiple tags") {
        Permission permission("packages.commit.testing.community.aarch64");
        REQUIRE(permission.tags().size() == 5);
        REQUIRE(permission.tags()[0] == "packages");
        REQUIRE(permission.tags()[1] == "commit");
        REQUIRE(permission.tags()[2] == "testing");
        REQUIRE(permission.tags()[3] == "community");
        REQUIRE(permission.tags()[4] == "aarch64");
    }

    SECTION("Create permission with empty string") {
        Permission permission("");
        REQUIRE(permission.tags().size() == 1);
        REQUIRE(permission.tags()[0].empty());
    }
}
