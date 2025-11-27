/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <catch2/catch_test_macros.hpp>
#include <rfl/json.hpp>

#include "../core/AggregateReflector.hpp"
#include "../core/AggregateRoot.hpp"

using namespace percpptency;

class SimpleAggregate : public AggregateRoot<SimpleAggregate> {
public:
    struct Data {
        std::string id;
        std::string name;
        int value;
    };

    SimpleAggregate() = default;
    
    explicit SimpleAggregate(Data data) : m_data(std::move(data)) {}

    Data m_data;
};

class ValidatedAggregate : public AggregateRoot<ValidatedAggregate> {
public:
    struct Data {
        std::string id;
        std::string email;
        int age;
    };

    static ValidatedAggregate create_from_data(Data const& data) {
        if (data.age < 0 || data.age > 150) {
            throw std::runtime_error("Invalid age");
        }
        
        ValidatedAggregate agg;
        agg.m_data = data;
        return agg;
    }

private:
    ValidatedAggregate() = default;

public:
    Data m_data;
};

TEST_CASE("AggregateReflector serialization", "[percpptency][reflector]") {
    SECTION("Simple aggregate - JSON round trip") {
        SimpleAggregate original;
        original.m_data = {"id-1", "test", 42};

        auto json = rfl::json::write(original);
        auto result = rfl::json::read<SimpleAggregate>(json);

        REQUIRE(result.value().m_data.id == "id-1");
        REQUIRE(result.value().m_data.name == "test");
        REQUIRE(result.value().m_data.value == 42);
    }

    SECTION("Validated aggregate - create_from_data factory") {
        ValidatedAggregate::Data data {"user-1", "test@example.com", 30};
        
        auto agg = ValidatedAggregate::create_from_data(data);
        REQUIRE(agg.m_data.age == 30);
    }

    SECTION("Validated aggregate - JSON round trip") {
        ValidatedAggregate original = ValidatedAggregate::create_from_data(
            {"id-2", "valid@example.com", 25});

        auto json = rfl::json::write(original);
        auto result = rfl::json::read<ValidatedAggregate>(json);

        REQUIRE(result.value().m_data.id == "id-2");
        REQUIRE(result.value().m_data.email == "valid@example.com");
        REQUIRE(result.value().m_data.age == 25);
    }

    SECTION("Validated aggregate - validation on deserialization") {
        std::string invalid_json = R"({"id":"id-3","email":"test@test.com","age":200})";
        
        REQUIRE_THROWS(rfl::json::read<ValidatedAggregate>(invalid_json));
    }

    SECTION("from_class extracts data") {
        SimpleAggregate agg;
        agg.m_data = {"extract-1", "extracted", 99};

        auto data = rfl::Reflector<SimpleAggregate>::from_class(agg);
        
        REQUIRE(data.id == "extract-1");
        REQUIRE(data.name == "extracted");
        REQUIRE(data.value == 99);
    }

    SECTION("to_class constructs aggregate") {
        SimpleAggregate::Data data {"construct-1", "constructed", 77};
        
        auto agg = rfl::Reflector<SimpleAggregate>::to_class(data);
        
        REQUIRE(agg.m_data.id == "construct-1");
        REQUIRE(agg.m_data.name == "constructed");
        REQUIRE(agg.m_data.value == 77);
    }
}

TEST_CASE("AggregateReflector concepts", "[percpptency][reflector][concepts]") {
    SECTION("AggregateWithData concept") {
        STATIC_REQUIRE(AggregateWithData<SimpleAggregate>);
        STATIC_REQUIRE(AggregateWithData<ValidatedAggregate>);
    }

    SECTION("ConstructibleFromData concept") {
        STATIC_REQUIRE(ConstructibleFromData<SimpleAggregate>);
        STATIC_REQUIRE_FALSE(ConstructibleFromData<ValidatedAggregate>);
    }

    SECTION("HasCreateFromData concept") {
        STATIC_REQUIRE_FALSE(HasCreateFromData<SimpleAggregate>);
        STATIC_REQUIRE(HasCreateFromData<ValidatedAggregate>);
    }
}
