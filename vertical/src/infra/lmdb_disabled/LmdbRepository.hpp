/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <iterator>
#include <memory>
#include <optional>

#include <boost/di.hpp>
#include <coro/coro.hpp>
#include <coro/io_scheduler.hpp>
#include <coro/task.hpp>
#include <coro/when_all.hpp>

#include "LmdbDatabase.hpp"
#include "LmdbEnvironment.hpp"
#include "shared/common.hpp"
#include "shared/domain/RepositoryBase.hpp"
#include "core/error/kinds.hpp"
#include "core/error/kinds.hpp"

namespace bxt::adapters::lmdb {

template<typename TEntity>
class LmdbRepository : public bxt::domain::RepositoryBase<TEntity, std::string> {
    using CrudError = err::CrudError;

public:
    static ResultTask<std::shared_ptr<LmdbRepository>, CrudError>
        create(std::shared_ptr<Environment> environment, std::string_view database_name) {
        auto db_result = co_await Database<TEntity>::open(environment, database_name);
        if (!db_result.has_value()) {
            co_return wrap_error<CrudError>(db_result.error(), earg(CrudError::Kind::DatabaseError),
                                            "Failed to open database");
        }
        co_return std::shared_ptr<LmdbRepository>(new LmdbRepository(*db_result));
    }

    BOOST_DI_INJECT(LmdbRepository,
                    std::shared_ptr<Environment> environment,
                    std::string_view database_name)
        : m_db(environment, database_name) {
    }

    ResultTask<TEntity, CrudError> get_by_id_async(std::string id) override {
        auto txn = co_await m_db.env()->begin_ro_txn();
        auto result = m_db.get(*txn, id);

        if (!result.has_value()) {
            co_return wrap_error<CrudError>(result.error(), earg(CrudError::Kind::DatabaseError),
                                            "Failed to get entity");
        }

        co_return *result;
    }

    ResultTask<Generator<TEntity>, CrudError>
        all_async(std::optional<Predicate<TEntity>> predicate = std::nullopt) override {
        auto txn = co_await m_db.env()->begin_ro_txn();
        auto result = m_db.find_by_predicate(*txn, predicate);

        if (!result.has_value()) {
            co_return wrap_error<CrudError>(result.error(), earg(CrudError::Kind::DatabaseError),
                                            "Failed to find entity");
        }
        co_return result;
    }

    ResultTask<std::size_t, CrudError>
        count_async(std::optional<Predicate<TEntity>> predicate = std::nullopt) override {
        auto txn = co_await m_db.env()->begin_ro_txn();

        if (predicate) {
            auto entries = co_await all_async(predicate);
            if (!entries.has_value()) {
                co_return wrap_error<CrudError>(entries.error(),
                                                earg(CrudError::Kind::DatabaseError),
                                                "Failed to count entities");
            }

            co_return std::ranges::distance(*entries);
        }

        auto stat = m_db.stat(*txn);
        if (!stat.has_value()) {
            co_return wrap_error<CrudError>(stat.error(), earg(CrudError::Kind::DatabaseError),
                                            "Failed to get database statistics");
        }
        co_return stat.value().ms_entries;
    }

    ResultTask<TEntity, CrudError> create_async(TEntity const& entity) override {
        auto txn = co_await m_db.env()->begin_rw_txn();
        auto result = m_db.put(*txn, bxt::to_string(entity.id()), entity);

        if (!result.has_value()) {
            co_return wrap_error<CrudError>(result.error(), earg(CrudError::Kind::DatabaseError),
                                            "Failed to create entity");
        }

        txn->commit();

        co_return entity;
    }

    ResultTask<TEntity, CrudError> update_async(TEntity const& entity) override {
        auto txn = co_await m_db.env()->begin_rw_txn();
        auto result = m_db.put(*txn, bxt::to_string(entity.id()), entity);

        if (!result.has_value()) {
            co_return wrap_error<CrudError>(result.error(), earg(CrudError::Kind::DatabaseError),
                                            "Failed to update entity");
        }

        txn->commit();

        co_return entity;
    }

    ResultTask<bool, CrudError> delete_async(std::string id) override {
        auto txn = co_await m_db.env()->begin_rw_txn();
        auto result = m_db.del(*txn, id);

        if (!result.has_value()) {
            co_return wrap_error<CrudError>(result.error(), earg(CrudError::Kind::DatabaseError),
                                            "Failed to delete entity");
        }

        txn->commit();

        co_return true;
    }

private:
    Database<TEntity> m_db;

    explicit LmdbRepository(Database<TEntity> db)
        : m_db(std::move(db)) {
    }
};

} // namespace bxt::adapters::lmdb
