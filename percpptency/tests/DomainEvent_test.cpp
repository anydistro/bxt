/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <catch2/catch_test_macros.hpp>

#include "../core/DomainEvent.hpp"

using namespace percpptency;

struct TestEvent : DomainEvent {
    std::string payload;
    int value;
};

TEST_CASE("DomainEvent basic functionality", "[percpptency][event]") {
    SECTION("Event has timestamp") {
        TestEvent event {"test", 42};
        
        REQUIRE(event.occurred_at.time_since_epoch().count() > 0);
    }

    SECTION("Event has unique ID") {
        TestEvent event1 {"test1", 1};
        TestEvent event2 {"test2", 2};
        
        REQUIRE_FALSE(event1.event_id.empty());
        REQUIRE_FALSE(event2.event_id.empty());
        REQUIRE(event1.event_id != event2.event_id);
    }

    SECTION("Events are copyable") {
        TestEvent original {"original", 100};
        TestEvent copy = original;
        
        REQUIRE(copy.payload == original.payload);
        REQUIRE(copy.value == original.value);
        REQUIRE(copy.event_id == original.event_id);
        REQUIRE(copy.occurred_at == original.occurred_at);
    }

    SECTION("Events are movable") {
        TestEvent original {"movable", 200};
        auto id = original.event_id;
        TestEvent moved = std::move(original);
        
        REQUIRE(moved.payload == "movable");
        REQUIRE(moved.value == 200);
        REQUIRE(moved.event_id == id);
    }
}
