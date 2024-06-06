/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/repositories/UnitOfWorkBase.h"
#include "frozen/string.h"
#include "frozen/unordered_map.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <coro/sync_wait.hpp>
#include <coro/task.hpp>
#include <coro/when_all.hpp>
#include <expected>
#include <functional>
#include <vector>

namespace bxt::Core::Domain {

struct ReadError : public bxt::Error {
    enum Type {
        EntityNotFound,
        EntityFindError,
        InvalidArgument,
    };

    ReadError() = default;

    ReadError(Type error_type) : error_type(error_type) {
        message = error_messages.at(error_type).data();
    }

    Type error_type;

private:
    static inline frozen::unordered_map<Type, frozen::string, 3>
        error_messages = {
            {Type::EntityNotFound, "Entity not found"},
            {Type::EntityFindError, "Error finding entity"},
            {Type::InvalidArgument, "Invalid argument"},
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
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<TResult> A task that represents the asynchronous
     * operation. The task's result is the found entity or an empty optional if
     * no entity was found.
     */
    virtual coro::task<TResult>
        find_by_id_async(TId id, std::shared_ptr<UnitOfWorkBase> uow) = 0;

    /**
     * @brief Asynchronously finds the first entity that satisfies a given
     * condition.
     * @param condition A function that specifies the condition to be satisfied.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<TResult> A task that represents the asynchronous
     * operation. The task's result is the found entity or an empty optional if
     * no entity was found.
     */
    virtual coro::task<TResult>
        find_first_async(std::function<bool(const TEntity&)>,
                         std::shared_ptr<UnitOfWorkBase> uow) = 0;

    /**
     * @brief Asynchronously finds all entities that satisfy a given condition.
     * @param condition A function that specifies the condition to be satisfied.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<TResults> A task that represents the asynchronous
     * operation. The task's result is a vector of the found entities or an
     * empty vector if no entity was found.
     */
    virtual coro::task<TResults>
        find_async(std::function<bool(const TEntity&)> condition,
                   std::shared_ptr<UnitOfWorkBase> uow) = 0;

    /**
     * @brief Asynchronously finds all entities stored in the repository.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<TResults> A task that represents the asynchronous
     * operation. The task's result is a vector of all entities stored in the
     * repository.
     */
    virtual coro::task<TResults>
        all_async(std::shared_ptr<UnitOfWorkBase> uow) = 0;
};
} // namespace bxt::Core::Domain
