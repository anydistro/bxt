
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

#include "../LmdbRepository.hpp"
#include "Setup.hpp"

struct Starship {
    std::string id() const {
        return m_id;
    }
    std::string captain_name;
    std::string m_id;
};

TEST_CASE("LmdbRepository basic operations", "[lmdb][repository]") {
    using namespace bxt;
    using namespace adapters::lmdb;

    auto [env, scheduler] = TestSetup::create_test_environment("LmdbRepository_test");

    SECTION("Repository CRUD operations") {
        auto repository_result =
            coro::sync_wait(LmdbRepository<Starship>::create(env, "test_repository"));

        REQUIRE(repository_result.has_value());

        auto repository = repository_result.value();

        SECTION("Create and Find operations") {
            Starship entity {"Captain Zelyony", "pegasus"};

            // Test create
            auto create_result = coro::sync_wait(repository->create_async(entity));
            REQUIRE(create_result.has_value());
            REQUIRE(create_result.value().captain_name == "Captain Zelyony");

            // Test find by id
            auto find_result = coro::sync_wait(repository->get_by_id_async("pegasus"));
            REQUIRE(find_result.has_value());
            REQUIRE(find_result.value().captain_name == "Captain Zelyony");
        }

        SECTION("Update operation") {
            Starship entity {"Captain Kim", "space_shuttle"};

            // Create initial entity
            coro::sync_wait(repository->create_async(entity));

            // Update entity
            entity.captain_name = "Professor Seleznyov";
            auto update_result = coro::sync_wait(repository->update_async(entity));
            REQUIRE(update_result.has_value());
            REQUIRE(update_result.value().captain_name == "Professor Seleznyov");

            // Verify update
            auto find_result = coro::sync_wait(repository->get_by_id_async("space_shuttle"));
            REQUIRE(find_result.has_value());

            REQUIRE(find_result.value().captain_name == "Professor Seleznyov");
        }

        SECTION("Delete operation") {
            Starship entity {"Gromozeka", "govorun"};

            // Create entity to delete
            coro::sync_wait(repository->create_async(entity));

            // Delete entity
            auto delete_result = coro::sync_wait(repository->delete_async("govorun"));
            REQUIRE(delete_result.has_value());
            REQUIRE(delete_result.value() == true);

            // Verify deletion
            auto find_result = coro::sync_wait(repository->get_by_id_async("govorun"));
            REQUIRE(!find_result.has_value());
        }

        SECTION("Find by predicate") {
            // Create multiple entities

            Starship entity1 {"Captain Buran", "buran"};
            Starship entity2 {"Robot Grrr", "grrr"};
            Starship entity3 {"Bird Govorun", "bird"};

            coro::sync_wait(repository->create_async(entity1));
            coro::sync_wait(repository->create_async(entity2));
            coro::sync_wait(repository->create_async(entity3));

            // Test predicate search
            auto predicate = [](Starship const& entity) {
                return entity.captain_name.find("Captain") != std::string::npos;
            };
            auto predicate_result = coro::sync_wait(repository->all_async(predicate));
            REQUIRE(predicate_result.has_value());

            int count = 0;
            for (auto&& found : predicate_result.value()) {
                REQUIRE(found.captain_name.find("Captain") != std::string::npos);
                count++;
            }
            REQUIRE(count == 1);
        }
    }

    env->env().close();
    TestSetup::cleanup_environment("LmdbRepository_test");
}
