/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "UnitOfWorkBase.h"
#include "core/domain/events/EventBase.h"
#include "frozen/unordered_map.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <any>
#include <coro/sync_wait.hpp>
#include <coro/task.hpp>
#include <coro/when_all.hpp>
#include <expected>
#include <functional>
#include <span>
#include <vector>

namespace bxt::Core::Domain {
struct ReadError : public bxt::Error {
    enum class ErrorTypes {
        EntityNotFound,
        EntityFindError,
        InvalidArgument,
    };

    ReadError() = default;

    ReadError(ErrorTypes error_type) : error_type(error_type) {}

    ReadError(ErrorTypes error_type, const bxt::Error&& source)
        : bxt::Error(std::make_unique<bxt::Error>(std::move(source))),
          error_type(error_type) {
        message = error_messages.at(error_type).data();
    }

    ErrorTypes error_type;

private:
    static inline frozen::unordered_map<ErrorTypes, std::string_view, 3>
        error_messages = {
            {ErrorTypes::EntityNotFound, "Entity not found"},
            {ErrorTypes::EntityFindError, "Error finding entity"},
            {ErrorTypes::InvalidArgument, "Invalid argument"},
        };
};

/**
 * @class ReadOnlyRepositoryBase
 *
 * @brief A base class for read-only repositories, that provides asynchronous
 * and synchronous methods for querying data.
 *
 * @tparam TEntity The entity type that the repository works with.
 */
template<typename TEntity> struct ReadOnlyRepositoryBase {
    BXT_DECLARE_RESULT(ReadError)

    /**
     * @typedef TId
     * @brief The type of the identifier of the entity.
     */
    using TId = std::remove_cvref_t<
        std::invoke_result_t<decltype(&TEntity::id), TEntity>>;

    /**
     * @typedef TResult
     * @brief The type that represents a single result of a query.
     */
    using TResult = Result<TEntity>;

    /**
     * @typedef TResults
     * @brief The type that represents multiple results of a query.
     */
    using TResults = Result<std::vector<TEntity>>;

    virtual ~ReadOnlyRepositoryBase() = default;

    /**
     * @brief Asynchronously finds an entity by its identifier.
     * @param id The identifier of the entity to find.
     * @return coro::task<TResult> A task that represents the asynchronous
     * operation. The task's result is the found entity or an empty optional if
     * no entity was found.
     */
    virtual coro::task<TResult> find_by_id_async(TId id) = 0;

    /**
     * @brief Synchronously finds an entity by its identifier.
     * @param id The identifier of the entity to find.
     * @return TResult The found entity or an empty optional if no entity was
     * found.
     */
    virtual TResult find_by_id(const TId& id) {
        return coro::sync_wait(find_by_id_async(id));
    }

    /**
     * @brief Asynchronously finds the first entity that satisfies a given
     * condition.
     * @param condition A function that specifies the condition to be satisfied.
     * @return coro::task<TResult> A task that represents the asynchronous
     * operation. The task's result is the found entity or an empty optional if
     * no entity was found.
     */
    virtual coro::task<TResult>
        find_first_async(std::function<bool(const TEntity&)>) = 0;

    /**
     * @brief Synchronously finds the first entity that satisfies a given
     * condition.
     * @param condition A function that specifies the condition to be satisfied.
     * @return TResult The found entity or an empty optional if no entity was
     * found.
     */
    virtual TResult
        find_first(const std::function<bool(const TEntity&)>& condition) {
        return coro::sync_wait(find_first_async(condition));
    }

    /**
     * @brief Asynchronously finds all entities that satisfy a given condition.
     * @param condition A function that specifies the condition to be satisfied.
     * @return coro::task<TResults> A task that represents the asynchronous
     * operation. The task's result is a vector of the found entities or an
     * empty vector if no entity was found.
     */
    virtual coro::task<TResults>
        find_async(std::function<bool(const TEntity&)> condition) = 0;

    /**
     * @brief Synchronously finds all entities that satisfy a given condition.
     *
     * @param condition A function that specifies the condition to be satisfied.
     * @return TResults A vector of the found entities or an empty vector if no
     * entity was found.
     */
    virtual TResults
        find(const std::function<bool(const TEntity&)>& condition) {
        return coro::sync_wait(find_async(condition));
    }

    /**
     * @brief Asynchronously finds all entities stored in the repository.
     *
     * @return coro::task<TResults> A task that represents the asynchronous
     * operation. The task's result is a vector of all entities stored in the
     * repository.
     */
    virtual coro::task<TResults> all_async() = 0;

    /**
     * @brief Synchronously finds all entities stored in the repository.
     *
     * @return TResults A vector of all entities stored in the repository.
     */
    virtual TResults all() { return coro::sync_wait(all_async()); }
};

struct WriteError : public bxt::Error {
    enum class ErrorTypes { EntityNotFound, OperationError, InvalidArgument };

    WriteError(ErrorTypes error_type) : error_type(error_type) {}

    WriteError(ErrorTypes error_type, const bxt::Error&& source)
        : bxt::Error(std::make_unique<bxt::Error>(std::move(source))),
          error_type(error_type) {
        message = error_messages.at(error_type).data();
    }

    ErrorTypes error_type;

private:
    static inline frozen::unordered_map<ErrorTypes, std::string_view, 3>
        error_messages = {
            {ErrorTypes::EntityNotFound, "Entity not found"},
            {ErrorTypes::OperationError, "Operation error"},
            {ErrorTypes::InvalidArgument, "Invalid argument"},
        };
};

/**
 * @class ReadWriteRepositoryBase
 *
 * @brief A base class for read-write repositories.
 *
 * A read-write repository is a data store that supports both reading and
 * writing of entities.
 *
 * This class extends ReadOnlyRepositoryBase by adding methods for adding,
 * updating, and removing entities.
 *
 * @tparam TEntity The type of the entities stored in the repository.
 */
template<typename TEntity>
struct ReadWriteRepositoryBase : public ReadOnlyRepositoryBase<TEntity>,
                                 public UnitOfWorkBase {
    using TId = typename ReadOnlyRepositoryBase<TEntity>::TId;
    using TResult = typename ReadOnlyRepositoryBase<TEntity>::TResult;
    using TResults = typename ReadOnlyRepositoryBase<TEntity>::TResults;
    using TEntities = std::vector<TEntity>;

    BXT_DECLARE_RESULT(WriteError)

    virtual ~ReadWriteRepositoryBase() = default;

    /**
     * @brief Asynchronously adds a new entity to the repository.
     *
     * @param entity The entity to be added.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>> add_async(const TEntity entity) = 0;

    /**
     * @brief Synchronously adds a new entity to the repository.
     *
     * @param entity The entity to be added.
     */
    virtual Result<void> add(const TEntity& entity) {
        return coro::sync_wait(add_async(entity));
    }

    /**
     * @brief Asynchronously adds a list of new entities to the repository.
     *
     * @param entities The entities to be added.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>> add_async(const TEntities entities) {
        std::vector<coro::task<Result<void>>> tasks;
        for (const auto& entity : entities) {
            tasks.push_back(add_async(entity));
        }

        co_await coro::when_all(std::move(tasks));
        co_return {};
    }

    /**
     * @brief Synchronously adds a list of new entities to the repository.
     *
     * @param entities The entities to be added.
     */
    virtual Result<void> add(const TEntities& entities) {
        return coro::sync_wait(add_async(entities));
    }

    /**
     * @brief Asynchronously updates an existing entity in the repository.
     *
     * @param entity The entity to be updated.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>> update_async(const TEntity entity) = 0;

    /**
     * @brief Synchronously updates an existing entity in the repository.
     *
     * @param entity The entity to be updated.
     */
    virtual Result<void> update(const TEntity& entity) {
        return coro::sync_wait(update_async(entity));
    }

    /**
     * @brief Asynchronously updates a list of existing entities in the
     * repository.
     *
     * @param entities The entities to be updated.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>> update_async(const TEntities entities) {
        std::vector<coro::task<Result<void>>> tasks;
        for (auto& entity : entities) {
            tasks.push_back(update_async(entity));
        }

        co_await coro::when_all(std::move(tasks));
        co_return {};
    }

    /**
     * @brief Synchronously updates a list of existing entities in the
     * repository.
     *
     * @param entities The entities to be updated.
     */
    virtual Result<void> update(const TEntities& entities) {
        return coro::sync_wait(update_async(entities));
    }

    /**
     * @brief Asynchronously removes an entity from the repository.
     *
     * @param id The ID of the entity to be removed.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>> remove_async(const TId id) = 0;

    /**
     * @brief Synchronously removes an entity from the repository.
     *
     * @param id The ID of the entity to be removed.
     */
    virtual Result<void> remove(const TId& id) {
        coro::sync_wait(remove_async(TId(id)));
    }

    /**
     * @brief Asynchronously removes a list of entities from the repository.
     *
     * @param ids The IDs of the entities to be removed.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>> remove_async(const std::vector<TId> ids) {
        std::vector<coro::task<Result<void>>> tasks;
        for (auto& id : ids) {
            tasks.push_back(remove_async(id));
        }

        co_await coro::when_all(std::move(tasks));
        co_return {};
    }

    /**
     * @brief Synchronously removes a list of entities from the repository.
     *
     * @param ids The IDs of the entities to be removed.
     */
    virtual Result<void> remove(const std::vector<TId>& ids) {
        return coro::sync_wait(remove_async(ids));
    }

protected:
    std::vector<Events::EventPtr> m_event_store;
};
} // namespace bxt::Core::Domain
