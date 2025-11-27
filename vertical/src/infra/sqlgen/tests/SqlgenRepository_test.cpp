/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <coro/io_scheduler.hpp>
#include <coro/sync_wait.hpp>
#include <rfl/json.hpp>

#include "../SqlgenRepository.hpp"
#include "../SqlgenSettings.hpp"
#include "../SqlgenUnitOfWork.hpp"
#include "../SqlgenUnitOfWorkFactory.hpp"

struct Starship {
    std::string id() const {
        return m_id;
    }
    std::string captain_name;
    std::string m_id;
};

namespace rfl {
template<> struct Reflector<Starship> {
    struct ReflType {
        std::string id;
        std::string captain_name;
    };

    static rfl::Result<Starship> to(ReflType const& refl) noexcept {
        return Starship {refl.captain_name, refl.id};
    }

    static ReflType from(Starship const& entity) {
        return ReflType {entity.id(), entity.captain_name};
    }
};
} // namespace rfl

class TestSetup {
public:
    static std::tuple<bxt::adapters::sqlgen::SqlgenSettings, std::shared_ptr<coro::io_scheduler>>
        create_test_environment(std::string const& test_name) {
        auto db_path = std::filesystem::temp_directory_path() / (test_name + ".db");

        bxt::adapters::sqlgen::SqlgenSettings settings;
        settings.connection_string = db_path.string();

        auto scheduler = coro::io_scheduler::make_shared();

        return {settings, std::move(scheduler)};
    }

    static void cleanup_environment(std::string const& test_name) {
        auto db_path = std::filesystem::temp_directory_path() / (test_name + ".db");
        std::filesystem::remove(db_path);
    }
};

TEST_CASE("SqlgenRepository basic operations", "[sqlgen][repository]") {
    using namespace bxt;
    using namespace adapters::sqlgen;

    auto [settings, scheduler] = TestSetup::create_test_environment("SqlgenRepository_test");
    SqlgenUnitOfWorkFactory factory(settings);

    SECTION("Repository CRUD operations") {
        SECTION("Create and Find operations") {
            auto [uow, error, repository] = coro::sync_wait(
                factory.with_rw_repositories<Starship>(SqlgenUnitOfWork::AutoCommit));

            REQUIRE(!error.has_value());
            REQUIRE(uow != nullptr);

            Starship entity {"Captain Zelyony", "pegasus"};

            // Test create
            auto create_result = repository.create(entity);
            REQUIRE(create_result.has_value());
            REQUIRE(create_result.value().captain_name == "Captain Zelyony");

            // Test find by id
            auto find_result = repository.get_by_id("pegasus");
            REQUIRE(find_result.has_value());
            REQUIRE(find_result.value().captain_name == "Captain Zelyony");
        }

        SECTION("Update operation") {
            auto [uow, error, repository] = coro::sync_wait(
                factory.with_rw_repositories<Starship>(SqlgenUnitOfWork::AutoCommit));

            REQUIRE(!error.has_value());
            REQUIRE(uow != nullptr);

            Starship entity {"Captain Kim", "space_shuttle"};

            // Create initial entity
            repository.create(entity);

            // Update entity
            entity.captain_name = "Professor Seleznyov";
            auto update_result = repository.update(entity);
            REQUIRE(update_result.has_value());
            REQUIRE(update_result.value().captain_name == "Professor Seleznyov");

            // Verify update
            auto find_result = repository.get_by_id("space_shuttle");
            REQUIRE(find_result.has_value());
            REQUIRE(find_result.value().captain_name == "Professor Seleznyov");
        }

        SECTION("Delete operation") {
            auto [uow, error, repository] = coro::sync_wait(
                factory.with_rw_repositories<Starship>(SqlgenUnitOfWork::AutoCommit));

            REQUIRE(!error.has_value());
            REQUIRE(uow != nullptr);

            Starship entity {"Gromozeka", "govorun"};

            // Create entity to delete
            repository.create(entity);

            // Delete entity
            auto delete_result = repository.delete_by_id("govorun");
            REQUIRE(delete_result.has_value());
            REQUIRE(delete_result.value() == true);

            // Verify deletion
            auto find_result = repository.get_by_id("govorun");
            REQUIRE(!find_result.has_value());
        }

        SECTION("Find all and count operations") {
            auto [uow, error, repository] = coro::sync_wait(
                factory.with_rw_repositories<Starship>(SqlgenUnitOfWork::AutoCommit));

            REQUIRE(!error.has_value());
            REQUIRE(uow != nullptr);

            // Create multiple entities
            Starship entity1 {"Captain Buran", "buran"};
            Starship entity2 {"Robot Grrr", "grrr"};
            Starship entity3 {"Bird Govorun", "bird"};

            repository.create(entity1);
            repository.create(entity2);
            repository.create(entity3);

            // Test find all
            auto all_result = repository.all();
            REQUIRE(all_result.has_value());
            REQUIRE(all_result.value().size() == 3);

            // Test count
            auto count_result = repository.count();
            REQUIRE(count_result.has_value());
            REQUIRE(count_result.value() == 3);
        }
    }

    SECTION("Unit of Work operations") {
        SECTION("Manual commit") {
            auto [uow, error, repository] = coro::sync_wait(
                factory.with_rw_repositories<Starship>(SqlgenUnitOfWork::ManualCommit));

            REQUIRE(!error.has_value());
            REQUIRE(uow != nullptr);

            Starship entity {"Captain Manual", "manual"};

            // Create entity
            auto create_result = repository.create(entity);
            REQUIRE(create_result.has_value());

            // Commit transaction
            coro::sync_wait(uow->commit());

            // Verify entity exists after commit
            auto find_result = repository.get_by_id("manual");
            REQUIRE(find_result.has_value());
            REQUIRE(find_result.value().captain_name == "Captain Manual");
        }

        SECTION("Rollback") {
            auto [uow, error, repository] = coro::sync_wait(
                factory.with_rw_repositories<Starship>(SqlgenUnitOfWork::ManualCommit));

            REQUIRE(!error.has_value());
            REQUIRE(uow != nullptr);

            Starship entity {"Captain Rollback", "rollback"};

            // Create entity
            auto create_result = repository.create(entity);
            REQUIRE(create_result.has_value());

            // Rollback transaction
            coro::sync_wait(uow->rollback());

            // Verify entity doesn't exist after rollback
            auto find_result = repository.get_by_id("rollback");
            REQUIRE(!find_result.has_value());
        }
    }

    SECTION("Multiple repositories in single unit of work") {
        auto [uow, error, starship_repo, second_repo] = coro::sync_wait(
            factory.with_rw_repositories<Starship, Starship>(SqlgenUnitOfWork::AutoCommit));

        REQUIRE(!error.has_value());
        REQUIRE(uow != nullptr);

        Starship entity1 {"Captain Multi1", "multi1"};
        Starship entity2 {"Captain Multi2", "multi2"};

        // Create entities using different repositories

        auto create1_result = starship_repo.create(entity1);
        auto create2_result = second_repo.create(entity2);

        REQUIRE(create1_result.has_value());
        REQUIRE(create2_result.has_value());

        // Verify both entities exist

        auto find1_result = starship_repo.get_by_id("multi1");
        auto find2_result = second_repo.get_by_id("multi2");

        REQUIRE(find1_result.has_value());
        REQUIRE(find2_result.has_value());
        REQUIRE(find1_result.value().captain_name == "Captain Multi1");
        REQUIRE(find2_result.value().captain_name == "Captain Multi2");
    }

    SECTION("Error handling") {
        SECTION("Non-existent entity") {
            auto [uow, error, repository] = coro::sync_wait(
                factory.with_ro_repositories<Starship>(SqlgenUnitOfWork::AutoCommit));

            REQUIRE(!error.has_value());
            REQUIRE(uow != nullptr);

            auto find_result = repository.get_by_id("non_existent");
            REQUIRE(!find_result.has_value());
        }

        SECTION("Delete non-existent entity") {
            auto [uow, error, repository] = coro::sync_wait(
                factory.with_rw_repositories<Starship>(SqlgenUnitOfWork::AutoCommit));

            REQUIRE(!error.has_value());
            REQUIRE(uow != nullptr);

            auto delete_result = repository.delete_by_id("non_existent");
            REQUIRE(delete_result.has_value());
            REQUIRE(delete_result.value() == true);
        }
    }

    TestSetup::cleanup_environment("SqlgenRepository_test");
}
