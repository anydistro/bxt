/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/entities/User.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::Core::Domain;

TEST_CASE("User entity", "[core][domain][entities]") {
    SECTION("Construction and getters") {
        User user(Name("testuser"), "password123");

        REQUIRE(user.name() == Name("testuser"));
        REQUIRE(user.password() == "password123");
        REQUIRE(user.permissions().empty());
    }

    SECTION("Setters") {
        User user(Name("testuser"), "password123");

        user.set_name("newuser");
        REQUIRE(user.name() == Name("newuser"));

        user.set_password("newpassword");
        REQUIRE(user.password() == "newpassword");

        std::set<Permission> new_permissions = {Permission("read"), Permission("write")};
        user.set_permissions(new_permissions);
        REQUIRE(user.permissions() == new_permissions);
    }

    SECTION("ID") {
        User user(Name("testuser"), "password123");
        REQUIRE(user.id() == Name("testuser"));
    }

    SECTION("Permission checking") {
        User user(Name("testuser"), "password123");
        std::set<Permission> permissions = {Permission("sections.*.*.*"),
                                            Permission("packages.get.stable.core.x86_64")};
        user.set_permissions(permissions);

        REQUIRE(user.has_permission("sections.stable.core.x86_64"));
        REQUIRE(user.has_permission("packages.get.stable.core.x86_64"));
        REQUIRE_FALSE(user.has_permission("packages.snap.stable.core.x86_64"));
        REQUIRE_FALSE(user.has_permission("users.add"));
    }

    SECTION("Default constructor") {
        User default_user;
        REQUIRE(default_user.name() == Name("Unnamed"));
        REQUIRE(default_user.password().empty());
        REQUIRE(default_user.permissions().empty());
    }
}
