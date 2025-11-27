/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <rfl.hpp>
#include <rfl/Variant.hpp>
#include <sqlgen/parsing/Parser.hpp>

#include "../SqlgenReflectorParser.hpp"
#include "../StringReflector.hpp"
#include "Setup.hpp"
#include "shared/common.hpp"
#include "core/error/kinds.hpp"

TEST_CASE("HasReflector Concept Tests", "[reflect][concept]") {
    SECTION("Concept correctly identifies types with reflectors") {
        REQUIRE(sqlgen::parsing::HasReflector<SimpleTestType>);
        REQUIRE(sqlgen::parsing::HasReflector<ComplexTestType>);
        REQUIRE(sqlgen::parsing::HasReflector<ErrorTestType>);
        REQUIRE(sqlgen::parsing::HasReflector<bxt::TimePoint>);
    }

    SECTION("Concept correctly rejects types without reflectors") {
        REQUIRE_FALSE(sqlgen::parsing::HasReflector<int>);
        REQUIRE_FALSE(sqlgen::parsing::HasReflector<std::string>);
        REQUIRE_FALSE(sqlgen::parsing::HasReflector<double>);
    }
}

TEST_CASE("SqlgenReflectorParser Tests", "[reflect][sqlgen]") {
    using Parser = sqlgen::parsing::Parser<SimpleTestType>;

    SECTION("Successful read from string") {
        auto result = Parser::read(std::string("parser_test"));
        REQUIRE(result.has_value());
        REQUIRE(result.value().value == "parser_test");
    }

    SECTION("Successful read from nullopt") {
        auto result = Parser::read(std::nullopt);
        // This should delegate to the underlying string parser behavior
        REQUIRE(!result.has_value());
    }

    SECTION("Successful write to string") {
        SimpleTestType test_obj {"write_test"};
        auto result = Parser::write(test_obj);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == "write_test");
    }

    SECTION("Error handling in read") {
        using ErrorParser = sqlgen::parsing::Parser<ErrorTestType>;
        auto result = ErrorParser::read(std::string("error"));
        REQUIRE(!result.has_value());
    }

    SECTION("SQL type delegation") {
        auto sql_type = Parser::to_type();
        REQUIRE(rfl::holds_alternative<sqlgen::dynamic::types::Text>(sql_type.variant()));
    }
}

TEST_CASE("Complex SqlgenReflectorParser Tests", "[reflect][sqlgen][complex]") {
    using ComplexParser = sqlgen::parsing::Parser<ComplexTestType>;

    SECTION("Round-trip through parser") {
        constexpr int TEST_NUMBER = 123;
        ComplexTestType original {TEST_NUMBER, "test_data"};

        auto write_result = ComplexParser::write(original);
        REQUIRE(write_result.has_value());
        REQUIRE(write_result.value() == "123:test_data");

        auto read_result = ComplexParser::read(write_result.value());
        REQUIRE(read_result.has_value());
        REQUIRE(read_result.value() == original);
    }

    SECTION("Parser error handling") {
        auto result = ComplexParser::read(std::string("invalid:format:too:many:colons"));
        REQUIRE(!result.has_value());
    }
}

TEST_CASE("TimePoint Integration Tests", "[reflect][timepoint]") {
    using namespace std::string_view_literals;
    using TimeParser = sqlgen::parsing::Parser<bxt::TimePoint>;

    SECTION("Valid ISO8601 through reflector") {
        auto time_str = "2024-01-01T12:00:00Z";
        auto result = TimeParser::read(std::string(time_str));
        REQUIRE(result.has_value());

        auto write_result = TimeParser::write(result.value());
        REQUIRE(write_result.has_value());
        REQUIRE(write_result.value() == time_str);
    }

    SECTION("Invalid timepoint through reflector") {
        auto result = TimeParser::read(std::string("invalid_time"));
        REQUIRE(!result.has_value());
    }

    SECTION("TimePoint SQL type") {
        auto sql_type = TimeParser::to_type();
        REQUIRE(rfl::holds_alternative<sqlgen::dynamic::types::Text>(sql_type.variant()));
    }
}

TEST_CASE("Edge Cases and Error Scenarios", "[reflect][edge_cases]") {
    SECTION("Empty string handling") {
        using Parser = sqlgen::parsing::Parser<SimpleTestType>;
        auto result = Parser::read(std::string(""));
        REQUIRE(result.has_value());
        REQUIRE(result.value().value == "");
    }

    SECTION("Very long string handling") {
        constexpr size_t LONG_STRING_LENGTH = 10000;
        std::string long_string(LONG_STRING_LENGTH, 'x');
        using Parser = sqlgen::parsing::Parser<SimpleTestType>;

        auto result = Parser::read(long_string);
        REQUIRE(result.has_value());
        REQUIRE(result.value().value == long_string);

        auto write_result = Parser::write(result.value());
        REQUIRE(write_result.has_value());
        REQUIRE(write_result.value() == long_string);
    }

    SECTION("Exception safety in conversion") {
        using ErrorParser = sqlgen::parsing::Parser<ErrorTestType>;

        // This should not throw, but return an error result
        auto read_result = ErrorParser::read(std::string("error"));
        REQUIRE(!read_result.has_value());

        // Write should still work for valid objects
        ErrorTestType valid_obj {"valid"};
        auto write_result = ErrorParser::write(valid_obj);
        REQUIRE(write_result.has_value());
        REQUIRE(write_result.value() == "valid");
    }

    SECTION("Parser type traits verification") {
        using Parser = sqlgen::parsing::Parser<ComplexTestType>;

        // Verify the parser uses the correct types
        REQUIRE(std::is_same_v<Parser::Type, ComplexTestType>);
    }

    SECTION("Concept requirements verification") {
        // These should all compile and pass due to HasReflector concept
        REQUIRE(sqlgen::parsing::HasReflector<SimpleTestType>);
        REQUIRE(sqlgen::parsing::HasReflector<ComplexTestType>);
        REQUIRE(sqlgen::parsing::HasReflector<ErrorTestType>);

        // Test that the concept correctly identifies required methods
        using TestReflector = rfl::Reflector<SimpleTestType>;
        REQUIRE(std::is_same_v<TestReflector::Type, SimpleTestType>);
        REQUIRE(std::is_same_v<TestReflector::ReflType, std::string>);
    }
}
