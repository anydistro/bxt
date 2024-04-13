/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/UserDTO.h"
#include "core/domain/entities/PackageLogEntry.h"
#include "core/domain/entities/PackageUpdateLogEntry.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "coro/task.hpp"
#include "coro/when_all.hpp"
#include "utilities/Error.h"
#include "utilities/StaticDTOMapper.h"
#include "utilities/errors/DatabaseError.h"
#include "utilities/lmdb/Database.h"
#include "utilities/lmdb/Environment.h"

#include <coro/coro.hpp>
#include <vector>

namespace bxt::Persistence {

template<typename TEntity, typename TDTO>
class LmdbRepositoryBase
    : public bxt::Core::Domain::ReadWriteRepositoryBase<TEntity> {
public:
    LmdbRepositoryBase(
        std::shared_ptr<bxt::Utilities::LMDB::Environment> environment,
        const std::string &database_name)
        : m_environment(environment), m_db(m_environment, database_name) {}

    using TResult =
        typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TResult;
    using TResults =
        typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TResults;
    using TId =
        typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TId;

    using TEntities =
        typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TEntities;

    template<typename T>
    using WriteResult = typename bxt::Core::Domain::ReadWriteRepositoryBase<
        TEntity>::template Result<T>;

    using TMapper = Utilities::StaticDTOMapper<TEntity, TDTO>;

    virtual coro::task<TResult> find_by_id_async(TId id) override {
        using namespace bxt::Core::Domain;

        const auto entity = co_await m_db.get(std::string(id));

        if (!entity.has_value()) {
            co_return bxt::make_error<ReadError>(
                ReadError::ErrorTypes::EntityNotFound);
        }

        co_return TMapper::to_entity(*entity);
    }
    virtual coro::task<TResult>
        find_first_async(std::function<bool(const TEntity &)>) override {}
    virtual coro::task<TResults>
        find_async(std::function<bool(const TEntity &)> condition) override {}
    virtual coro::task<TResults> all_async() override {
        TEntities results;

        co_await m_db.accept(
            [&results]([[maybe_unused]] std::string_view key, const TDTO &e) {
                results.push_back(TMapper::to_entity(e));
                return Utilities::NavigationAction::Next;
            });

        co_return results;
    }

    virtual coro::task<WriteResult<void>>
        add_async(const TEntity entity) override {
        m_to_add.push_back(entity);
        co_return {};
    }
    virtual coro::task<WriteResult<void>>
        update_async(const TEntity entity) override {
        m_to_update.push_back(entity);
        co_return {};
    }
    virtual coro::task<WriteResult<void>> remove_async(const TId id) override {
        m_to_remove.push_back(id);
        co_return {};
    }

    virtual coro::task<UnitOfWorkBase::Result<void>> commit_async() override {
        std::vector<coro::task<typename decltype(m_db)::template Result<bool>>>
            tasks;

        for (const auto &el : m_to_add) {
            tasks.push_back(
                m_db.put(std::string(el.id()), TMapper::to_dto(el)));
        }
        for (const auto &el : m_to_remove) {
            tasks.push_back(m_db.del(std::string(el)));
        }

        co_await coro::when_all(std::move(tasks));

        m_to_add.clear();
        m_to_remove.clear();
        m_to_update.clear();

        co_return {};
    }
    virtual coro::task<UnitOfWorkBase::Result<void>> rollback_async() override {
        m_to_add.clear();
        m_to_remove.clear();
        m_to_update.clear();

        co_return {};
    }

private:
    std::shared_ptr<bxt::Utilities::LMDB::Environment> m_environment;
    Utilities::LMDB::Database<TDTO> m_db;

    std::vector<TEntity> m_to_add;
    std::vector<TId> m_to_remove;
    std::vector<TEntity> m_to_update;
};

} // namespace bxt::Persistence
