/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <rfl.hpp>

#include "../StringReflector.hpp"
#include "Setup.hpp"

TEST_CASE("StringReflectorBase Tests", "[reflect][string_reflector]") {
    using SimpleReflector = bxt::reflect::StringReflectorBase<SimpleTestType>;

    SECTION("Type definitions") {
        REQUIRE(std::is_same_v<SimpleReflector::Type, SimpleTestType>);
        REQUIRE(std::is_same_v<SimpleReflector::ReflType, std::string>);
    }

    SECTION("Successful conversion from string") {
        auto result = SimpleReflector::to("test_value");
        REQUIRE(result.has_value());
        REQUIRE(result.value().value == "test_value");
    }

    SECTION("Successful conversion to string") {
        SimpleTestType test_obj {"hello_world"};
        auto result = SimpleReflector::from(test_obj);
        REQUIRE(result == "hello_world");
    }

    SECTION("Error handling in conversion") {
        using ErrorReflector = bxt::reflect::StringReflectorBase<ErrorTestType>;
        auto result = ErrorReflector::to("error");
        REQUIRE(!result.has_value());
        REQUIRE(result.error().what().find("Intentional error") != std::string::npos);
    }

    SECTION("SQL type generation") {
        auto sql_type = SimpleReflector::to_type();
        REQUIRE(rfl::holds_alternative<sqlgen::dynamic::types::Text>(sql_type.variant()));
    }
}

TEST_CASE("Complex Type String Reflection", "[reflect][complex_types]") {
    using ComplexReflector = bxt::reflect::StringReflectorBase<ComplexTestType>;

    SECTION("Round-trip conversion") {
        ComplexTestType original {42, "hello"};
        auto string_repr = ComplexReflector::from(original);
        REQUIRE(string_repr == "42:hello");

        auto result = ComplexReflector::to(string_repr);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == original);
    }

    SECTION("Invalid format handling") {
        auto result = ComplexReflector::to("invalid_format");
        REQUIRE(!result.has_value());
    }

    SECTION("Non-numeric data handling") {
        auto result = ComplexReflector::to("abc:hello");
        REQUIRE(!result.has_value());
    }
}

TEST_CASE("Macro Generated Reflector Tests", "[reflect][macro]") {
    SECTION("rfl::Reflector specialization exists") {
        REQUIRE(std::is_same_v<rfl::Reflector<SimpleTestType>::Type, SimpleTestType>);
        REQUIRE(std::is_same_v<rfl::Reflector<SimpleTestType>::ReflType, std::string>);
    }

    SECTION("Reflector methods work correctly") {
        SimpleTestType test_obj {"macro_test"};

        auto string_result = rfl::Reflector<SimpleTestType>::from(test_obj);
        REQUIRE(string_result == "macro_test");

        auto obj_result = rfl::Reflector<SimpleTestType>::to(string_result);
        REQUIRE(obj_result.has_value());
        REQUIRE(obj_result.value() == test_obj);
    }
}
