/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/entities/Package.h"

#include <catch2/catch_test_macros.hpp>

using namespace bxt::Core::Domain;

TEST_CASE("Package", "[core][domain][entities]") {
    SECTION("Parse file name") {
        REQUIRE(Package::parse_file_name("package-1.0.0-1-x86_64.pkg.tar.zst").value()
                == "package");
        REQUIRE(Package::parse_file_name("package-1.0.0-1-any.pkg.tar.zst").value() == "package");
        REQUIRE(Package::parse_file_name("lib32-package-1.0.0-1-x86_64.pkg.tar.zst").value()
                == "lib32-package");

        // Invalid cases
        REQUIRE(Package::parse_file_name("package.pkg.tar.zst") == std::nullopt);
        REQUIRE(Package::parse_file_name("1.0.0-1-x86_64.pkg.tar.zst") == std::nullopt);
        REQUIRE(Package::parse_file_name("package-") == std::nullopt);
        REQUIRE(Package::parse_file_name("package-1.0.0-1.pkg.tar.zst") == std::nullopt);
    }
}
