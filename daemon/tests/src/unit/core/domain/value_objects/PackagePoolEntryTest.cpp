
/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/value_objects/PackagePoolEntry.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

using namespace bxt::Core::Domain;

TEST_CASE("PackagePoolEntry", "[core][domain][value_objects]") {
    SECTION("Parse valid file path") {
        auto cwd = std::filesystem::absolute(".");
        std::filesystem::path file_path = "data/dummy-1-1-any.pkg.tar.zst";
        auto result = PackagePoolEntry::parse_file_path(file_path, std::nullopt);
        REQUIRE(result.has_value());
        auto entry = result.value();
        REQUIRE(entry.file_path() == file_path);
        REQUIRE(entry.version().string() == "1-1");
    }

    SECTION("Parse file path with signature") {
        std::filesystem::path file_path = "data/dummy-1-1-any.pkg.tar.zst";
        std::filesystem::path sig_path = "data/dummy-1-1-any.pkg.tar.zst.sig";
        auto result = PackagePoolEntry::parse_file_path(file_path, sig_path);
        REQUIRE(result.has_value());
        auto entry = result.value();
        REQUIRE(entry.file_path() == file_path);
        REQUIRE(entry.signature_path() == sig_path);
    }

    SECTION("Parse invalid file path") {
        std::filesystem::path file_path = "invalid-package-name";
        auto result = PackagePoolEntry::parse_file_path(file_path, std::nullopt);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error().error_code
                == PackagePoolEntry::ParsingError::ErrorCode::InvalidFilename);
    }

    SECTION("Accessors") {
        std::filesystem::path file_path = "data/dummy-1-1-x86_64.pkg.tar.zst";
        std::filesystem::path sig_path = "data/dummy-1-1-x86_64.pkg.tar.zst.sig";
        bxt::Utilities::AlpmDb::Desc desc;
        PackageVersion version = PackageVersion::from_string("1.0.0-1").value();

        PackagePoolEntry entry(file_path, sig_path, desc, version);

        REQUIRE(entry.file_path() == file_path);
        REQUIRE(entry.signature_path() == sig_path);
        REQUIRE(entry.version().string() == version.string());
        REQUIRE(entry.desc().desc == desc.desc);
        REQUIRE(entry.desc().files == desc.files);
    }
}
