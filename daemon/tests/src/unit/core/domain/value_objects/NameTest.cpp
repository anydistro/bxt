
/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/value_objects/Name.h"

#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>

using namespace bxt::Core::Domain;

TEST_CASE("Name", "[core][domain][value_objects]") {
    SECTION("Construction and string conversion") {
        Name name("TestName");
        REQUIRE(static_cast<std::string>(name) == "TestName");
    }

    SECTION("Empty name throws exception") {
        REQUIRE_THROWS(Name(""));
    }

    SECTION("Comparison") {
        Name name1("Name1");
        Name name2("Name1");
        Name name3("Name2");

        REQUIRE(name1 == name2);
        REQUIRE(name1 != name3);
        REQUIRE(name1 < name3);
        REQUIRE(name3 > name1);
    }

    SECTION("Formatting") {
        Name name("FormatTest");
        std::string formatted = fmt::format("{}", name);
        REQUIRE(formatted == "FormatTest");
    }
}
