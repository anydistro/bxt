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
#include <lmdbxx/lmdb++.h>

#include "../LmdbDatabase.hpp"
#include "Setup.hpp"

TEST_CASE("LmdbDatabase basic operations", "[lmdb][database]") {
    using namespace bxt;
    using namespace adapters::lmdb;

    auto [env, scheduler] = TestSetup::create_test_environment("LmdbDatabase_test");

    SECTION("Database creation and CRUD operations") {
        // Open method will create transaction itself

        SECTION("Put and Get operations") {
            auto db_result =
                coro::sync_wait(Database<int>::open(env, "LmdbDatabase_test::db_crud"));

            REQUIRE(db_result.has_value());
            auto db = db_result.value();

            // Test putting a value with a new transaction
            auto put_txn = coro::sync_wait(env->begin_rw_txn());
            auto put_result = db.put(*put_txn, "test_key", 42);
            REQUIRE(put_result.has_value());
            REQUIRE(put_result.value());
            (*put_txn).commit();
            put_txn.unlock();

            // Test getting the value back with a read-only transaction
            auto get_txn = coro::sync_wait(env->begin_ro_txn());
            auto get_result = db.get(*get_txn, "test_key");
            REQUIRE(get_result.has_value());
            REQUIRE(get_result.value() == 42);
            get_txn.unlock();

            // Test getting non-existent key with a read-only transaction
            auto missing_txn = coro::sync_wait(env->begin_ro_txn());
            auto missing_result = db.get(*missing_txn, "nonexistent");
            REQUIRE_FALSE(missing_result.has_value());
            REQUIRE(missing_result.error().get().kind == err::CrudError::Kind::NotFound);
            missing_txn.unlock();
        }

        SECTION("Delete operation") {
            auto db_delete_result =
                coro::sync_wait(Database<int>::open(env, "LmdbDatabase_test::db_delete"));
            REQUIRE(db_delete_result.has_value());
            auto db_delete = db_delete_result.value();

            // Insert with a new transaction
            auto insert_txn = coro::sync_wait(env->begin_rw_txn());
            auto insert_result = db_delete.put(*insert_txn, "delete_key", 100);
            REQUIRE(insert_result.has_value());
            (*insert_txn).commit();
            insert_txn.unlock();

            // Delete with a new transaction
            auto del_txn = coro::sync_wait(env->begin_rw_txn());
            auto del_result = db_delete.del(*del_txn, "delete_key");
            REQUIRE(del_result.has_value());
            REQUIRE(del_result.value());
            (*del_txn).commit();
            del_txn.unlock();

            // Verify deletion with a read-only transaction
            auto verify_txn = coro::sync_wait(env->begin_ro_txn());
            auto get_after_del = db_delete.get(*verify_txn, "delete_key");
            REQUIRE_FALSE(get_after_del.has_value());
            REQUIRE(get_after_del.error().get().kind == err::CrudError::Kind::NotFound);
            verify_txn.unlock();
        }

        SECTION("Find all operation") {
            auto db_find_result =
                coro::sync_wait(Database<int>::open(env, "LmdbDatabase_test::db_find"));
            REQUIRE(db_find_result.has_value());
            auto db_find = db_find_result.value();

            // Insert multiple values with separate transactions
            {
                auto txn1 = coro::sync_wait(env->begin_rw_txn());
                db_find.put(*txn1, "key1", 1);
                (*txn1).commit();
                txn1.unlock();
            }
            {
                auto txn2 = coro::sync_wait(env->begin_rw_txn());
                db_find.put(*txn2, "key2", 2);
                (*txn2).commit();
                txn2.unlock();
            }
            {
                auto txn3 = coro::sync_wait(env->begin_rw_txn());
                db_find.put(*txn3, "prefix_key3", 3);
                (*txn3).commit();
                txn3.unlock();
            }
            {
                auto txn4 = coro::sync_wait(env->begin_rw_txn());
                db_find.put(*txn4, "prefix_key4", 4);
                (*txn4).commit();
                txn4.unlock();
            }

            // Test finding all entries with a read-only transaction
            auto find_all_txn = coro::sync_wait(env->begin_ro_txn());
            auto find_all_result = db_find.find_all(*find_all_txn);
            REQUIRE(find_all_result.has_value());

            int count = 0;
            for (auto _ : find_all_result.value()) {
                count++;
            }
            REQUIRE(count == 4);
            find_all_txn.unlock();

            // Test finding with prefix with a read-only transaction
            auto find_prefix_txn = coro::sync_wait(env->begin_ro_txn());
            auto find_prefix_result = db_find.find_all(*find_prefix_txn, "prefix_");
            REQUIRE(find_prefix_result.has_value());

            count = 0;
            for (auto&& [key, value] : find_prefix_result.value()) {
                REQUIRE(std::string_view(key).starts_with("prefix_"));
                count++;
            }
            REQUIRE(count == 2);
            find_prefix_txn.unlock();
        }

        SECTION("Find by predicate") {
            auto db_predicate_result =
                coro::sync_wait(Database<int>::open(env, "LmdbDatabase_test::db_predicate"));
            REQUIRE(db_predicate_result.has_value());
            auto db_predicate = db_predicate_result.value();

            constexpr int kValue1 = 10;
            constexpr int kValue2 = 20;
            constexpr int kValue3 = 30;
            constexpr int kThreshold = 15;

            // Insert test data with separate transactions
            {
                auto txn1 = coro::sync_wait(env->begin_rw_txn());
                db_predicate.put(*txn1, "key1", kValue1);
                txn1->commit();
                txn1.unlock();
            }
            {
                auto txn2 = coro::sync_wait(env->begin_rw_txn());
                db_predicate.put(*txn2, "key2", kValue2);
                txn2->commit();
                txn2.unlock();
            }
            {
                auto txn3 = coro::sync_wait(env->begin_rw_txn());
                db_predicate.put(*txn3, "key3", kValue3);
                txn3->commit();
                txn3.unlock();
            }

            // Test predicate with a new transaction
            auto predicate_txn = coro::sync_wait(env->begin_rw_txn());
            auto predicate = [](int const& value) { return value > kThreshold; };
            auto predicate_result = db_predicate.find_by_predicate(*predicate_txn, predicate);
            REQUIRE(predicate_result.has_value());

            int count = 0;
            for (auto&& value : predicate_result.value()) {
                REQUIRE(value > kThreshold);
                count++;
            }
            REQUIRE(count == 2);
            predicate_txn->commit();
            predicate_txn.unlock();
        }
    }

    env->env().close();

    TestSetup::cleanup_environment("LmdbDatabase_test");
}
