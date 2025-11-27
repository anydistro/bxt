/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <catch2/catch_test_macros.hpp>

#include "../core/AggregateRoot.hpp"

using namespace percpptency;

struct UserCreated : DomainEvent {
    std::string user_id;
    std::string name;
};

struct PasswordChanged : DomainEvent {
    std::string user_id;
};

class TestAggregate : public AggregateRoot<TestAggregate> {
public:
    struct Data {
        std::string id;
        std::string name;
        std::string password;
    };

    TestAggregate() = default;

    explicit TestAggregate(std::string id, std::string name) {
        m_data.id = std::move(id);
        m_data.name = std::move(name);
        
        record_event(UserCreated {m_data.id, m_data.name});
    }

    void change_password(std::string new_password) {
        m_data.password = std::move(new_password);
        record_event(PasswordChanged {m_data.id});
    }

    Data m_data;
};

TEST_CASE("AggregateRoot event tracking", "[percpptency][aggregate]") {
    SECTION("New aggregate has no events") {
        TestAggregate agg;
        REQUIRE(agg.uncommitted_events().empty());
    }

    SECTION("Recording events") {
        TestAggregate agg {"user-1", "Alice"};
        
        auto const& events = agg.uncommitted_events();
        REQUIRE(events.size() == 1);
    }

    SECTION("Multiple events tracked") {
        TestAggregate agg {"user-2", "Bob"};
        agg.change_password("new-secret");
        
        auto const& events = agg.uncommitted_events();
        REQUIRE(events.size() == 2);
    }

    SECTION("Take uncommitted events clears them") {
        TestAggregate agg {"user-3", "Charlie"};
        agg.change_password("password123");
        
        REQUIRE(agg.uncommitted_events().size() == 2);
        
        auto events = agg.take_uncommitted_events();
        REQUIRE(events.size() == 2);
        REQUIRE(agg.uncommitted_events().empty());
    }

    SECTION("Mark events as committed clears them") {
        TestAggregate agg {"user-4", "Diana"};
        agg.change_password("pass456");
        
        REQUIRE(agg.uncommitted_events().size() == 2);
        
        agg.mark_events_as_committed();
        REQUIRE(agg.uncommitted_events().empty());
    }

    SECTION("Aggregates are copyable") {
        TestAggregate original {"user-5", "Eve"};
        original.change_password("secret");
        
        TestAggregate copy = original;
        REQUIRE(copy.m_data.id == original.m_data.id);
        REQUIRE(copy.m_data.name == original.m_data.name);
        REQUIRE(copy.uncommitted_events().size() == 2);
    }

    SECTION("Aggregates are movable") {
        TestAggregate original {"user-6", "Frank"};
        original.change_password("pass");
        
        TestAggregate moved = std::move(original);
        REQUIRE(moved.m_data.id == "user-6");
        REQUIRE(moved.uncommitted_events().size() == 2);
    }
}
