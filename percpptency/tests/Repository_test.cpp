/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include <catch2/catch_test_macros.hpp>

#include "../core/Repository.hpp"
#include "../core/AggregateRoot.hpp"

using namespace percpptency;

class TestEntity : public AggregateRoot<TestEntity> {
public:
    struct Data {
        std::string id;
        std::string name;
    };

    TestEntity() = default;
    explicit TestEntity(std::string id, std::string name) {
        m_data.id = std::move(id);
        m_data.name = std::move(name);
    }

    Data m_data;
};

class MockRepository : public Repository<TestEntity> {
public:
    std::vector<TestEntity> storage;

    ResultVoid create(TestEntity& entity) override {
        storage.push_back(entity);
        return {};
    }

    ResultVoid update(TestEntity& entity) override {
        for (auto& e : storage) {
            if (e.m_data.id == entity.m_data.id) {
                e = entity;
                return {};
            }
        }
        return std::unexpected(CrudError::NotFound);
    }

    Result<TestEntity, CrudError> get_by_id(std::string id) override {
        for (auto const& e : storage) {
            if (e.m_data.id == id) {
                return e;
            }
        }
        return std::unexpected(CrudError::NotFound);
    }

    Result<std::vector<TestEntity>, CrudError> all() override {
        return storage;
    }

    ResultVoid delete_by_id(std::string id) override {
        auto it = std::remove_if(storage.begin(), storage.end(),
                                 [&](auto const& e) { return e.m_data.id == id; });
        if (it != storage.end()) {
            storage.erase(it, storage.end());
            return {};
        }
        return std::unexpected(CrudError::NotFound);
    }

    Result<std::size_t, CrudError> count() override {
        return storage.size();
    }
};

TEST_CASE("Repository interface", "[percpptency][repository]") {
    MockRepository repo;

    SECTION("Create entity") {
        TestEntity entity {"1", "Alice"};
        auto result = repo.create(entity);
        
        REQUIRE(result.has_value());
        REQUIRE(repo.storage.size() == 1);
    }

    SECTION("Get by ID") {
        TestEntity entity {"2", "Bob"};
        repo.create(entity);
        
        auto result = repo.get_by_id("2");
        REQUIRE(result.has_value());
        REQUIRE(result.value().m_data.name == "Bob");
    }

    SECTION("Get non-existent entity") {
        auto result = repo.get_by_id("non-existent");
        REQUIRE_FALSE(result.has_value());
        REQUIRE(result.error() == CrudError::NotFound);
    }

    SECTION("Update entity") {
        TestEntity entity {"3", "Charlie"};
        repo.create(entity);
        
        entity.m_data.name = "Charles";
        auto result = repo.update(entity);
        
        REQUIRE(result.has_value());
        REQUIRE(repo.storage[0].m_data.name == "Charles");
    }

    SECTION("Delete entity") {
        TestEntity entity {"4", "Diana"};
        repo.create(entity);
        
        auto result = repo.delete_by_id("4");
        REQUIRE(result.has_value());
        REQUIRE(repo.storage.empty());
    }

    SECTION("Count entities") {
        repo.create(TestEntity {"5", "Eve"});
        repo.create(TestEntity {"6", "Frank"});
        
        auto result = repo.count();
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 2);
    }

    SECTION("Get all entities") {
        repo.create(TestEntity {"7", "Grace"});
        repo.create(TestEntity {"8", "Heidi"});
        
        auto result = repo.all();
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 2);
    }
}

TEST_CASE("CrudError to_string", "[percpptency][error]") {
    REQUIRE(std::string(to_string(CrudError::NotFound)) == "NotFound");
    REQUIRE(std::string(to_string(CrudError::AlreadyExists)) == "AlreadyExists");
    REQUIRE(std::string(to_string(CrudError::InvalidData)) == "InvalidData");
    REQUIRE(std::string(to_string(CrudError::ConstraintViolation)) == "ConstraintViolation");
    REQUIRE(std::string(to_string(CrudError::ConcurrencyConflict)) == "ConcurrencyConflict");
    REQUIRE(std::string(to_string(CrudError::StorageError)) == "StorageError");
}
