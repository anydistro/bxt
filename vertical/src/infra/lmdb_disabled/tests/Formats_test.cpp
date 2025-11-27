
/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#include <catch2/catch_test_macros.hpp>

#include "../internal/Formats.hpp"

namespace bxt::adapters::lmdb::tests {

struct TestStruct {
    int id;
    std::string name;
    double value;
};

TEST_CASE("Internal format serialization/deserialization", "[formats]") {
    SECTION("Primitive type serialization/deserialization") {
        int i = 42;
        auto serialized = internal::serialize(i);
        REQUIRE(!serialized.empty());

        auto deserialized = internal::deserialize<int>(serialized.data(), serialized.size());
        REQUIRE(deserialized.has_value());
        REQUIRE(deserialized.value() == i);
    }

    SECTION("Basic type serialization/deserialization") {
        TestStruct original {42, "test", 3.14};

        auto serialized = internal::serialize(original);
        REQUIRE(!serialized.empty());

        auto deserialized = internal::deserialize<TestStruct>(serialized.data(), serialized.size());

        REQUIRE(deserialized->id == original.id);
        REQUIRE(deserialized->name == original.name);
        REQUIRE(deserialized->value == original.value);
    }

    SECTION("Empty object serialization/deserialization") {
        TestStruct empty {0, "", 0.0};

        auto serialized = internal::serialize(empty);
        REQUIRE(!serialized.empty());

        auto deserialized = internal::deserialize<TestStruct>(serialized.data(), serialized.size());

        REQUIRE(deserialized->id == empty.id);
        REQUIRE(deserialized->name == empty.name);
        REQUIRE(deserialized->value == empty.value);
    }

    SECTION("Invalid data deserialization") {
        std::string invalid_data = "not valid serialized data";
        auto result = internal::deserialize<TestStruct>(invalid_data.data(), invalid_data.size());

        REQUIRE(!result.has_value());
    }
}

} // namespace bxt::adapters::lmdb::tests
