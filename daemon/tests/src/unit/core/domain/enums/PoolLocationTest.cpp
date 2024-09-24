/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "core/domain/enums/PoolLocation.h"

#include <catch2/catch_test_macros.hpp>
#include <map>

using namespace bxt::Core::Domain;

TEST_CASE("PoolLocation enum", "[core][domain][enums]") {
    SECTION("to_string conversion") {
        REQUIRE(bxt::to_string(PoolLocation::Sync) == "sync");
        REQUIRE(bxt::to_string(PoolLocation::Overlay) == "overlay");
        REQUIRE(bxt::to_string(PoolLocation::Automated) == "automated");
        REQUIRE_THROWS(bxt::to_string(PoolLocation::Unknown));
    }

    SECTION("select_preferred_pool_location") {
        SECTION("Empty map") {
            std::map<PoolLocation, int> empty_map;
            auto result = select_preferred_pool_location(empty_map);
            REQUIRE_FALSE(result.has_value());
        }

        SECTION("Single element map") {
            std::map<PoolLocation, int> single_map = {{PoolLocation::Sync, 1}};
            auto result = select_preferred_pool_location(single_map);
            REQUIRE(result.has_value());
            REQUIRE(*result == PoolLocation::Sync);
        }

        SECTION("Multiple elements map") {
            std::map<PoolLocation, int> multi_map = {
                {PoolLocation::Sync, 1}, {PoolLocation::Overlay, 2}, {PoolLocation::Automated, 3}};
            auto result = select_preferred_pool_location(multi_map);
            REQUIRE(result.has_value());
            REQUIRE(*result == PoolLocation::Overlay);
        }

        SECTION("Map with Unknown location") {
            std::map<PoolLocation, int> map_with_unknown = {
                {PoolLocation::Unknown, 0}, {PoolLocation::Automated, 1}, {PoolLocation::Sync, 2}};
            auto result = select_preferred_pool_location(map_with_unknown);
            REQUIRE(result.has_value());
            REQUIRE(*result == PoolLocation::Automated);
        }
    }
}
