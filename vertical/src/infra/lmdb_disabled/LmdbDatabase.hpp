/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <exception>
#include <memory>
#include <optional>
#include <string_view>

#include <boost/di.hpp>
#include <coro/sync_wait.hpp>
#include <lmdbxx/lmdb++.h>

#include "internal/Formats.hpp"
#include "LmdbCursorIterator.hpp"
#include "LmdbEnvironment.hpp"
#include "shared/common.hpp"
#include "core/error/kinds.hpp"
#include "core/error/Utils.hpp"

namespace bxt::adapters::lmdb {

namespace lmdbxx = ::lmdb;

template<typename TEntity> class Database {
public:
    using CrudError = bxt::err::CrudError;
    using KeyValue = std::pair<std::string_view, TEntity>;

    static Task<Result<Database<TEntity>, CrudError>> open(std::shared_ptr<Environment> env,
                                                           std::string_view name = "") {
        auto txn = co_await env->begin_rw_txn();
        Database result;
        result.m_env = env;

        result.m_dbi = name.empty() ? lmdbxx::dbi::open(*txn, nullptr, MDB_CREATE)
                                    : lmdbxx::dbi::open(*txn, name.data(), MDB_CREATE);

        txn->commit();
        co_return result;
    }

    BOOST_DI_INJECT(Database, std::shared_ptr<Environment> env, std::string_view name)
        : Database(coro::sync_wait(Database::open(env, name)).value()) {
    }

    Result<bool, CrudError> put(lmdbxx::txn& txn, std::string_view key, TEntity const& value) {
        return with_db_error<bool>(
            CrudError::Kind::DatabaseError, "put", [&]() -> Result<bool, CrudError> {
                auto serialized = internal::serialize<TEntity>(value);

                bool result =
                    m_dbi.put(txn, key, std::string_view {serialized.data(), serialized.size()});
                return result;
            });
    }

    Result<bool, CrudError> del(lmdbxx::txn& txn, std::string_view key) {
        return with_db_error<bool>(CrudError::Kind::DatabaseError, "del",
                                   [&]() -> Result<bool, CrudError> {
                                       bool result = m_dbi.del(txn, key);
                                       return result;
                                   });
    }

    Result<TEntity, CrudError> get(lmdbxx::txn& txn, std::string_view key) {
        return with_db_error<TEntity>(
            CrudError::Kind::DatabaseError, "get", [&]() -> Result<TEntity, CrudError> {
                std::string_view value_string;

                if (!m_dbi.get(txn, key, value_string)) {
                    return make_error<CrudError>(earg(CrudError::Kind::NotFound),
                                                 "Entity not found in database");
                }

                auto entity =
                    internal::deserialize<TEntity>(value_string.data(), value_string.size());

                if (!entity.has_value()) {
                    return make_error<CrudError>(earg(CrudError::Kind::InvalidData),
                                                 "Failed to deserialize entity");
                }

                return entity.value();
            });
    }
    GeneratorResult<KeyValue, CrudError> find_all(lmdbxx::txn& txn, std::string_view prefix = "") {
        return with_db_error<Generator<KeyValue>>(

            CrudError::Kind::DatabaseError, "find_all",
            [](lmdbxx::txn& t, lmdbxx::dbi& dbi, std::string_view p) -> Generator<KeyValue> {
                auto cursor = std::make_shared<lmdbxx::cursor>(lmdbxx::cursor::open(t, dbi));
                return make_find_all_generator(std::move(cursor), p);
            },
            txn, m_dbi, prefix);
    }

    GeneratorResult<TEntity, CrudError>
        find_by_predicate(lmdbxx::txn& txn,
                          std::optional<Predicate<TEntity>> predicate,
                          std::string_view prefix = "") {
        auto result = find_all(txn, prefix);

        if (!result.has_value()) {
            // Propagate the original CrudError without attempting to
            // reconstruct it from an Error<CrudError> instance.
            return bxt::err::convert_error<CrudError>(result.error());
        }

        return [](Generator<KeyValue> gen,
                  std::optional<Predicate<TEntity>> predicate) -> Generator<TEntity> {
            for (auto&& [key, value] : gen) {
                if (predicate && !(*predicate)(value)) {
                    continue;
                }
                co_yield value;
            }
        }(std::move(result.value()), std::move(predicate));
    }

    Result<MDB_stat, CrudError> stat(lmdbxx::txn& txn) {
        return with_db_error<MDB_stat>(
            CrudError::Kind::DatabaseError, "stat",
            [&]() -> Result<MDB_stat, CrudError> { return m_dbi.stat(txn); });
    }

    lmdbxx::dbi& dbi() noexcept {
        return m_dbi;
    }

    std::shared_ptr<Environment> env() noexcept {
        return m_env;
    }

private:
    explicit Database() = default;

    template<typename TReturn, typename F, typename... Args>
    Result<TReturn, CrudError>
        with_db_error(CrudError::Kind kind, std::string_view prefix, F&& f, Args&&... args) {
        try {
            return f(std::forward<Args>(args)...);
        } catch (lmdbxx::error const& err) {
            return make_error<CrudError>(earg(kind), std::string(prefix) + err.what());
        } catch (std::exception const& e) {
            return make_error<CrudError>(earg(kind), std::string(prefix) + e.what());
        }
    }

    // Pulls out the cursor‐walk logic so find_all() stays flat
    static Generator<std::pair<std::string_view, TEntity>>
        make_find_all_generator(std::shared_ptr<lmdbxx::cursor> cursor, std::string_view prefix) {
        using Iterator = LmdbCursorIterator<TEntity>;

        auto begin = prefix.empty() ? Iterator::begin(cursor) : Iterator::from_key(cursor, prefix);
        auto end = Iterator::end();

        for (auto it = begin; it != end; ++it) {
            if (!prefix.empty() && !(*it)->first.starts_with(prefix)) {
                break;
            }
            co_yield **it;
        }
    }

    std::shared_ptr<Environment> m_env;
    lmdbxx::dbi m_dbi;
};

} // namespace bxt::adapters::lmdb
