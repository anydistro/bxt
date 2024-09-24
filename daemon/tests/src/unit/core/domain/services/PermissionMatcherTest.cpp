/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/services/PermissionMatcher.h"

#include "core/domain/value_objects/Permission.h"

#include <catch2/catch_test_macros.hpp>
using namespace bxt::Core::Domain;

TEST_CASE("PermissionMatcher", "[core][domain][services]") {
    SECTION("Exact match") {
        Permission p1("a.b.c");
        Permission p2("a.b.c");
        REQUIRE(PermissionMatcher::match(p1, p2));
    }

    SECTION("Wildcard match") {
        Permission p1("a.*.c");
        Permission p2("a.b.c");
        REQUIRE(PermissionMatcher::match(p1, p2));
    }

    SECTION("Partial match with wildcard") {
        Permission p1("a.*");
        Permission p2("a.b.c");
        REQUIRE(PermissionMatcher::match(p1, p2));
    }

    SECTION("No match") {
        Permission p1("a.b.c");
        Permission p2("x.y.z");
        REQUIRE_FALSE(PermissionMatcher::match(p1, p2));
    }

    SECTION("Different length, no match") {
        Permission p1("a.b");
        Permission p2("a.b.c");
        REQUIRE_FALSE(PermissionMatcher::match(p1, p2));
    }

    SECTION("Different length with wildcard, match") {
        Permission p1("a.b.*");
        Permission p2("a.b.c.d");
        REQUIRE(PermissionMatcher::match(p1, p2));
    }

    SECTION("Empty permissions") {
        Permission p1("");
        Permission p2("");
        REQUIRE(PermissionMatcher::match(p1, p2));
    }

    SECTION("Single wildcard") {
        Permission p1("*");
        Permission p2("a.b.c");
        REQUIRE(PermissionMatcher::match(p1, p2));
    }
}
