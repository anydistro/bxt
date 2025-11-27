/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <cpperr.hpp>
#include "core/error/kinds.hpp"

using namespace bxt::err;
using namespace cpperr;

TEST_CASE("cpperr basic functionality", "[errors]") {
    SECTION("Make simple error") {
        auto err = make_error(CrudError::NotFound);
        REQUIRE(err.error().kind() == CrudError::NotFound);
        REQUIRE(std::string(err.error().what()).find("NotFound") != std::string::npos);
    }

    SECTION("Make error with context") {
        auto err = make_error(CrudError::NotFound, "Entity not in DB");
        REQUIRE(err.error().kind() == CrudError::NotFound);
        auto trace = err.error().trace();
        REQUIRE(trace.find("NotFound") != std::string::npos);
        REQUIRE(trace.find("Entity not in DB") != std::string::npos);
    }

    SECTION("Wrap error") {
        auto low_err = make_error(CrudError::NotFound, "Low level failure");
        auto high_err = wrap_error(ParseError::MissingField, low_err.error());
        
        REQUIRE(high_err.error().kind() == ParseError::MissingField);
        REQUIRE(high_err.error().caused_by<CrudError>());
        
        auto trace = high_err.error().trace();
        REQUIRE(trace.find("MissingField") != std::string::npos);
        REQUIRE(trace.find("NotFound") != std::string::npos);
    }

    SECTION("Auto-convert error with convert_error function") {
        auto crud_err = make_error(CrudError::InvalidData);
        auto parse_err = wrap_error<ParseError>(crud_err.error());
        
        REQUIRE(parse_err.error().kind() == ParseError::InvalidFormat);
        REQUIRE(parse_err.error().caused_by<CrudError>());
    }
}

TEST_CASE("cpperr TRY macros", "[errors]") {
    auto success_fn = []() -> result<int, CrudError> { return 42; };
    auto failure_fn = []() -> result<int, CrudError> {
        return make_error(CrudError::NotFound);
    };

    SECTION("ERR_TRY with success") {
        auto test_fn = [&]() -> result<int, CrudError> {
            auto value = ERR_TRY(success_fn());
            return value;
        };
        
        auto res = test_fn();
        REQUIRE(res.has_value());
        REQUIRE(*res == 42);
    }

    SECTION("ERR_TRY with failure") {
        auto test_fn = [&]() -> result<int, CrudError> {
            auto value = ERR_TRY(failure_fn());
            return value;  // Should never reach here
        };
        
        auto res = test_fn();
        REQUIRE(!res.has_value());
        REQUIRE(res.error().kind() == CrudError::NotFound);
    }

    SECTION("ERR_TRY_AUTO with conversion") {
        auto test_fn = [&]() -> result<int, ParseError> {
            auto value = ERR_TRY_AUTO(failure_fn(), ParseError);
            return value;
        };
        
        auto res = test_fn();
        REQUIRE(!res.has_value());
        // CrudError::NotFound converts to ParseError::MissingField
        REQUIRE(res.error().kind() == ParseError::MissingField);
        REQUIRE(res.error().caused_by<CrudError>());
    }
}

TEST_CASE("Error kind conversions", "[errors]") {
    SECTION("CrudError to ParseError") {
        REQUIRE(convert_error<ParseError>(CrudError::InvalidData) == ParseError::InvalidFormat);
        REQUIRE(convert_error<ParseError>(CrudError::NotFound) == ParseError::MissingField);
    }

    SECTION("ParseError to CrudError") {
        REQUIRE(convert_error<CrudError>(ParseError::InvalidFormat) == CrudError::InvalidData);
        REQUIRE(convert_error<CrudError>(ParseError::MissingField) == CrudError::NotFound);
    }
}
