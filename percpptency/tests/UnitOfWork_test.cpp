/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <catch2/catch_test_macros.hpp>

#include "../core/UnitOfWork.hpp"

using namespace percpptency;

struct TestEvent1 : DomainEvent {
    std::string data;
};

struct TestEvent2 : DomainEvent {
    int value;
};

class MockUnitOfWork : public UnitOfWork {
public:
    bool committed = false;
    bool rolled_back = false;

    coro::task<void> commit() override {
        committed = true;
        co_return;
    }

    coro::task<void> rollback() override {
        rolled_back = true;
        co_return;
    }
};

TEST_CASE("UnitOfWork event collection", "[percpptency][uow]") {
    SECTION("Empty UnitOfWork has no events") {
        MockUnitOfWork uow;
        REQUIRE(uow.uncommitted_events().empty());
    }

    SECTION("Add events to UnitOfWork") {
        MockUnitOfWork uow;
        
        std::vector<DomainEvent> events;
        events.push_back(TestEvent1 {"event1"});
        events.push_back(TestEvent2 {42});
        
        uow.add_events(std::move(events));
        
        REQUIRE(uow.uncommitted_events().size() == 2);
    }

    SECTION("Take uncommitted events") {
        MockUnitOfWork uow;
        
        std::vector<DomainEvent> events;
        events.push_back(TestEvent1 {"take-test"});
        uow.add_events(std::move(events));
        
        auto taken = uow.take_uncommitted_events();
        REQUIRE(taken.size() == 1);
        REQUIRE(uow.uncommitted_events().empty());
    }

    SECTION("Multiple event batches") {
        MockUnitOfWork uow;
        
        std::vector<DomainEvent> batch1;
        batch1.push_back(TestEvent1 {"batch1"});
        uow.add_events(std::move(batch1));
        
        std::vector<DomainEvent> batch2;
        batch2.push_back(TestEvent2 {100});
        batch2.push_back(TestEvent2 {200});
        uow.add_events(std::move(batch2));
        
        REQUIRE(uow.uncommitted_events().size() == 3);
    }
}

TEST_CASE("TransactionError to_string", "[percpptency][error]") {
    REQUIRE(std::string(to_string(TransactionError::BeginFailed)) == "BeginFailed");
    REQUIRE(std::string(to_string(TransactionError::CommitFailed)) == "CommitFailed");
    REQUIRE(std::string(to_string(TransactionError::RollbackFailed)) == "RollbackFailed");
    REQUIRE(std::string(to_string(TransactionError::ConcurrencyConflict)) == "ConcurrencyConflict");
    REQUIRE(std::string(to_string(TransactionError::ConnectionError)) == "ConnectionError");
}
