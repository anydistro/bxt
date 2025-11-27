
/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <coro/io_scheduler.hpp>
#include <coro/sync_wait.hpp>
#include <coro/when_all.hpp>
#include <lmdbxx/lmdb++.h>

#include "Setup.hpp"

TEST_CASE("LmdbEnvironment basic operations", "[lmdb][environment]") {
    using namespace bxt;
    using namespace adapters::lmdb;

    auto [env, scheduler] = TestSetup::create_test_environment("LmdbEnvironment_test");

    SECTION("Transaction management") {
        SECTION("Read-write transaction") {
            auto rw_txn = coro::sync_wait(env->begin_rw_txn());
            REQUIRE_NOTHROW((*rw_txn).commit());
            rw_txn.unlock();
        }

        SECTION("Read-only transaction") {
            auto ro_txn = coro::sync_wait(env->begin_ro_txn());
            REQUIRE_NOTHROW(ro_txn.unlock());
        }

        SECTION("Transaction abort") {
            auto rw_txn = coro::sync_wait(env->begin_rw_txn());
            REQUIRE_NOTHROW((*rw_txn).abort());
            rw_txn.unlock();
        }
    }

    SECTION("Concurrent transaction management") {
        SECTION("Concurrent read-only transactions") {
            auto test_fn_1 = [](auto env, auto scheduler) -> Task<void> {
                co_await scheduler->schedule();
                auto rw_txn1 = co_await env->begin_ro_txn();
                REQUIRE_NOTHROW(rw_txn1->commit());
                rw_txn1.unlock();
            };
            auto test_fn_2 = [](auto env, auto scheduler) -> Task<void> {
                co_await scheduler->schedule();
                auto rw_txn2 = co_await env->begin_ro_txn();
                REQUIRE_NOTHROW(rw_txn2->commit());
                rw_txn2.unlock();
            };

            REQUIRE_NOTHROW(coro::sync_wait(
                coro::when_all(test_fn_1(env, scheduler), test_fn_2(env, scheduler))));
        }
    }

    SECTION("Environment cleanup") {
        REQUIRE_NOTHROW(env->env().close());

        TestSetup::cleanup_environment("LmdbEnvironment_test");
    }
}
