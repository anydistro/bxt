/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/value_objects/PackageArchitecture.h"

#include <catch2/catch_test_macros.hpp>
#include <string>

using namespace bxt::Core::Domain;

TEST_CASE("PackageArchitecture value object", "[core][domain][value_objects]") {
    SECTION("Default constructor") {
        PackageArchitecture arch;
        REQUIRE(std::string(arch) == "any");
    }

    SECTION("Constructor with non-empty string") {
        PackageArchitecture arch("x86_64");
        REQUIRE(std::string(arch) == "x86_64");
    }

    SECTION("Constructor with empty string") {
        PackageArchitecture arch("");
        REQUIRE(std::string(arch) == "any");
    }

    SECTION("Implicit conversion to string") {
        PackageArchitecture arch("arm64");
        std::string arch_str = arch;
        REQUIRE(arch_str == "arm64");
    }

    SECTION("Comparison") {
        PackageArchitecture arch1("x86_64");
        PackageArchitecture arch2("x86_64");
        PackageArchitecture arch3("arm64");

        REQUIRE(std::string(arch1) == std::string(arch2));
        REQUIRE(std::string(arch1) != std::string(arch3));
    }
}
