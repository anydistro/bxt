/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/entities/Section.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::Core::Domain;

TEST_CASE("Section entity", "[core][domain][entities]") {
    SECTION("Construction and getters") {
        Section section(Name("stable"), Name("core"), Name("x86_64"));

        REQUIRE(section.branch() == Name("stable"));
        REQUIRE(section.repository() == Name("core"));
        REQUIRE(section.architecture() == Name("x86_64"));
    }

    SECTION("Setters") {
        Section section(Name("stable"), Name("core"), Name("x86_64"));

        section.set_branch(Name("testing"));
        REQUIRE(section.branch() == Name("testing"));

        section.set_repository(Name("extra"));
        REQUIRE(section.repository() == Name("extra"));

        section.set_architecture(Name("aarch64"));
        REQUIRE(section.architecture() == Name("aarch64"));
    }

    SECTION("ID generation") {
        Section section(Name("stable"), Name("core"), Name("x86_64"));
        REQUIRE(section.id() == "stable/core/x86_64");
    }

    SECTION("String representation") {
        Section section(Name("stable"), Name("core"), Name("x86_64"));
        REQUIRE(section.string() == "stable/core/x86_64");
    }

    SECTION("to_string function") {
        Section section(Name("stable"), Name("core"), Name("x86_64"));
        REQUIRE(bxt::to_string(section) == "stable/core/x86_64");
    }
}
