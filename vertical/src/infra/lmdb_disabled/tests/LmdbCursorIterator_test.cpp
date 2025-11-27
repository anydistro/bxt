/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <memory>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <lmdbxx/lmdb++.h>

#include "../LmdbCursorIterator.hpp"
#include "Setup.hpp"

TEST_CASE("LmdbCursorIterator basic operations", "[lmdb][iterator]") {
    using namespace bxt::adapters::lmdb;

    auto [env, scheduler] = TestSetup::create_test_environment("LmdbCursorIterator_test");
    auto txn = coro::sync_wait(env->begin_rw_txn());
    auto dbi = lmdbxx::dbi::open(*txn, "LmdbCursorIterator_test::test_db", MDB_CREATE);
    txn->commit();
    txn.unlock();

    SECTION("Iterator creation and navigation") {
        constexpr auto Key1TestData = "key1";
        constexpr auto Key2TestData = "key2";
        constexpr auto Test42 = "42";
        constexpr auto Test84 = "84";

        std::string key1 = Key1TestData;
        auto value1 = internal::serialize(std::string(Test42));
        std::string key2 = Key2TestData;
        auto value2 = internal::serialize(std::string(Test84));

        txn = coro::sync_wait(env->begin_rw_txn());
        dbi.put(*txn, key1, std::string_view {value1.data(), value1.size()});
        dbi.put(*txn, key2, std::string_view {value2.data(), value2.size()});
        txn->commit();
        txn.unlock();

        SECTION("Forward iteration") {
            txn = coro::sync_wait(env->begin_rw_txn());
            auto cursor = std::make_shared<lmdbxx::cursor>(lmdbxx::cursor::open(*txn, dbi));
            auto it = LmdbCursorIterator<std::string>::begin(cursor);
            auto end = LmdbCursorIterator<std::string>::end();

            REQUIRE(it != end);
            auto first = *it;

            REQUIRE(first.has_value());
            REQUIRE(first->first == Key1TestData);
            REQUIRE(first->second == Test42);

            ++it;
            REQUIRE(it != end);
            auto second = *it;

            REQUIRE(second.has_value());
            REQUIRE(second->first == Key2TestData);
            REQUIRE(second->second == Test84);

            ++it;
            REQUIRE(it == end);
            txn->commit();
            txn.unlock();
        }

        SECTION("Reverse iteration") {
            txn = coro::sync_wait(env->begin_rw_txn());
            auto cursor = std::make_shared<lmdbxx::cursor>(lmdbxx::cursor::open(*txn, dbi));
            auto it = LmdbCursorIterator<std::string>::last(cursor);
            auto end = LmdbCursorIterator<std::string>::end();

            REQUIRE(it != end);
            auto last = *it;

            REQUIRE(last.has_value());
            REQUIRE(last->first == Key2TestData);
            REQUIRE(last->second == Test84);

            --it;
            REQUIRE(it != end);
            auto first = *it;

            REQUIRE(first.has_value());
            REQUIRE(first->first == Key1TestData);
            REQUIRE(first->second == Test42);
            txn->commit();
            txn.unlock();
        }

        SECTION("Range-based iteration") {
            txn = coro::sync_wait(env->begin_rw_txn());
            auto cursor = std::make_shared<lmdbxx::cursor>(lmdbxx::cursor::open(*txn, dbi));
            auto it = LmdbCursorIterator<std::string>::from_key(cursor, Key1TestData);
            REQUIRE(it != LmdbCursorIterator<std::string>::end());
            auto value = *it;

            REQUIRE(value.has_value());
            REQUIRE(value->first == Key1TestData);
            REQUIRE(value->second == Test42);
            txn->commit();
            txn.unlock();
        }
    }

    env->env().close();
    TestSetup::cleanup_environment("LmdbCursorIterator_test");
}
