/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/events/EventBase.h"
#include "core/domain/repositories/ReadOnlyRepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "frozen/string.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <coro/sync_wait.hpp>
#include <coro/task.hpp>
#include <coro/when_all.hpp>
#include <expected>
#include <vector>

namespace bxt::Core::Domain {

struct WriteError : public bxt::Error {
    enum Type { EntityNotFound, OperationError, InvalidArgument };

    WriteError(Type error_type) : error_type(error_type) {
        message = error_messages.at(error_type).data();
    }

    Type error_type;

private:
    static inline frozen::unordered_map<Type, frozen::string, 3>
        error_messages = {
            {Type::EntityNotFound, "Entity not found"},
            {Type::OperationError, "Operation error"},
            {Type::InvalidArgument, "Invalid argument"},
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
struct ReadWriteRepositoryBase : public ReadOnlyRepositoryBase<TEntity> {
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
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>>
        add_async(const TEntity entity,
                  std::shared_ptr<UnitOfWorkBase> uow) = 0;

    /**
     * @brief Asynchronously adds a list of new entities to the repository.
     *
     * @param entities The entities to be added.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>>
        add_async(const TEntities entities,
                  std::shared_ptr<UnitOfWorkBase> uow) {
        auto tasks = entities | std::views::transform([&](const auto& entity) {
                         return add_async(entity, uow);
                     })
                     | std::ranges::to<std::vector>();

        auto results = co_await coro::when_all(std::move(tasks));

        for (const auto& result : results) {
            if (!result.return_value().has_value()) {
                co_return std::unexpected(result.return_value().error());
            }
        }

        co_return {};
    }

    /**
     * @brief Asynchronously updates an existing entity in the repository.
     *
     * @param entity The entity to be updated.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>>
        update_async(const TEntity entity,
                     std::shared_ptr<UnitOfWorkBase> uow) = 0;

    /**
     * @brief Asynchronously updates a list of existing entities in the
     * repository.
     *
     * @param entities The entities to be updated.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>>
        update_async(const TEntities entities,
                     std::shared_ptr<UnitOfWorkBase> uow) {
        auto tasks = entities | std::views::transform([&](const auto& entity) {
                         return update_async(entity, uow);
                     })
                     | std::ranges::to<std::vector>();

        auto results = co_await coro::when_all(std::move(tasks));
        for (const auto& result : results) {
            if (!result.return_value().has_value()) {
                co_return std::unexpected(result.return_value().error());
            }
        }
        co_return {};
    }

    /**
     * @brief Asynchronously saves an entity in the repository.
     * If the entity exists, it will be updated; otherwise, it will be added.
     *
     * @param entity The entity to be saved.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<Result<void>> A task that represents the asynchronous
     * operation.
     */
    inline virtual coro::task<Result<void>>
        save_async(const TEntity& entity, std::shared_ptr<UnitOfWorkBase> uow) {
        auto find_result = co_await this->find_by_id_async(entity.id(), uow);

        if (!find_result.has_value()) {
            if (find_result.error().error_type
                == ReadError::Type::EntityNotFound) {
                co_return co_await add_async(entity, uow);
            } else {
                co_return bxt::make_error_with_source<WriteError>(
                    std::move(find_result.error()), WriteError::OperationError);
            }
        }

        co_return co_await update_async(entity, uow);
    }

    /**
     * @brief Asynchronously saves a list of entities in the repository.
     * For each entity, if it exists, it will be updated; otherwise, it will be
     * added.
     *
     * @param entities The entities to be saved.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<Result<void>> A task that represents the asynchronous
     * operation.
     */
    inline virtual coro::task<Result<void>>
        save_async(const TEntities& entities,
                   std::shared_ptr<UnitOfWorkBase> uow) {
        auto tasks = entities
                     | std::views::transform([&](const TEntity& entity) {
                           return save_async(entity, uow);
                       })
                     | std::ranges::to<std::vector>();

        auto results = co_await coro::when_all(std::move(tasks));
        for (const auto& result : results) {
            if (!result.return_value().has_value()) {
                co_return std::unexpected(result.return_value().error());
            }
        }
        co_return {};
    }

    /**
     * @brief Asynchronously removes an entity from the repository.
     *
     * @param id The ID of the entity to be removed.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>>
        delete_async(const TId id, std::shared_ptr<UnitOfWorkBase> uow) = 0;

    /**
     * @brief Asynchronously removes a list of entities from the repository.
     *
     * @param ids The IDs of the entities to be removed.
     * @param uow The shared pointer to the unit of work.
     * @return coro::task<void> A task that represents the asynchronous
     * operation.
     */
    virtual coro::task<Result<void>>
        delete_async(const std::vector<TId> ids,
                     std::shared_ptr<UnitOfWorkBase> uow) {
        auto tasks = ids | std::views::transform([&](const auto& id) {
                         return delete_async(id, uow);
                     })
                     | std::ranges::to<std::vector>();

        auto results = co_await coro::when_all(std::move(tasks));
        for (const auto& result : results) {
            if (!result.return_value().has_value()) {
                co_return std::unexpected(result.return_value().error());
            }
        }
        co_return {};
    }

protected:
    std::vector<Events::EventPtr> m_event_store;
};

} // namespace bxt::Core::Domain
