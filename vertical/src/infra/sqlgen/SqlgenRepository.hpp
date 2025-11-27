/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023‒2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 */
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/di.hpp>
#include <rfl/Result.hpp>
#include <sqlgen/aggregations.hpp>
#include <sqlgen/create_table.hpp>
#include <sqlgen/insert.hpp>
#include <sqlgen/to.hpp>

#include "infra/reflect/SqlgenJsonParser.hpp"
#include "infra/reflect/SqlgenReflectorParser.hpp"
#include "core/result_types.hpp"
#include "core/domain/RepositoryBase.hpp"
#include "core/error/kinds.hpp"
#include "utils/ConvertTo.hpp"
#include "SqlgenError.hpp"
#include "SqlgenHeaders.hpp"
#include "SqlgenUnitOfWork.hpp"

namespace bxt::adapters::sqlgen {
using ::sqlgen::literals::operator""_c; // convenience for "id"_c etc.
using CrudError = err::CrudError;

template<typename T>
concept Reflectable = requires { typename rfl::Reflector<T>; };

template<typename T> using ReflType_t = typename rfl::Reflector<T>::ReflType;

// map any sqlgen error into CrudError + context
template<typename R>
[[nodiscard]] static auto map_crud(rfl::Error&& err, std::string_view ctx) {
    // Convert rfl::Error to SqlgenError first
    SqlgenError sqlgen_error(std::move(err));

    // Convert SqlgenError to CrudError using the conversion utility
    auto conversion_result = bxt::utils::convert_to<CrudError, SqlgenError> {}(sqlgen_error);
    
    CrudError error_kind;
    if (!conversion_result) {
        // Fallback to generic storage error if conversion fails
        error_kind = CrudError::StorageError;
    } else {
        error_kind = conversion_result.value();
    }

    // Return error<CrudError> directly (not wrapped in std::unexpected)
    // transform_error will wrap it for us
    return cpperr::error<CrudError>(error_kind, std::string(ctx));
}

template<Reflectable TEntity> class SqlgenRepository {
    using SerializationType = ReflType_t<TEntity>;

public:
    explicit SqlgenRepository(std::shared_ptr<SqlgenUnitOfWork> uow)
        : m_unit_of_work(std::move(uow)) {
        // Create table once; throw if anything fails.
        auto create_res = get_transaction().and_then([&](auto const& trx) {
            return trx->connection().and_then(::sqlgen::create_table<SerializationType>);
        });
        if (!create_res) {
            throw std::runtime_error(create_res.error().what());
        }
    }

    Result<TEntity, CrudError> get_by_id(std::string id) {
        return get_transaction()
            .and_then([id](auto const& trx) {
                return trx->connection().and_then(::sqlgen::read<SerializationType>
                                                  | ::sqlgen::where("id"_c == id)
                                                  | ::sqlgen::limit(1));
            })
            .and_then([id](SerializationType&& row) -> rfl::Result<TEntity> {
                return rfl::Reflector<TEntity>::to_class(row);
            })
            .transform_error([](auto&& err) {
                return map_crud<TEntity>(std::forward<decltype(err)>(err), "get_by_id failed");
            });
    }

    Result<std::vector<TEntity>, CrudError> all() {
        return get_transaction()
            .and_then([](auto const& trx) {
                auto query = ::sqlgen::read<std::vector<SerializationType>>;

                return trx->connection().and_then(query);
            })
            .and_then(
                [](std::vector<SerializationType>&& rows) -> rfl::Result<std::vector<TEntity>> {
                    std::vector<TEntity> out;
                    out.reserve(rows.size());
                    for (auto& r : rows) {
                        out.push_back(rfl::Reflector<TEntity>::to_class(r));
                    }
                    return out;
                })
            .transform_error([](auto&& err) {
                return map_crud<std::vector<TEntity>>(std::forward<decltype(err)>(err),
                                                      "all failed");
            });
    }

    Result<std::size_t, CrudError> count() {
        struct Cnt {
            std::size_t cnt = 0;
        };
        return get_transaction()
            .and_then([](auto const& trx) {
                return trx->connection().and_then(
                    ::sqlgen::select_from<SerializationType>(::sqlgen::count().as<"cnt">())
                    | ::sqlgen::to<std::vector<Cnt>>);
            })
            .and_then([](std::vector<Cnt>&& v) { return v.empty() ? 0 : v.front().cnt; })
            .transform_error([](auto&& err) {
                return map_crud<std::size_t>(std::forward<decltype(err)>(err), "count failed");
            });
    }

    Result<TEntity, CrudError> create(TEntity const& entity) {
        return get_transaction()
            .and_then([&entity](auto const& trx) {
                auto ser = rfl::Reflector<TEntity>::from_class(entity);
                return trx->connection().and_then(::sqlgen::insert(std::move(ser)));
            })
            .and_then([&entity](auto&&) -> rfl::Result<TEntity> { return entity; })
            .transform_error([](auto&& err) {
                return map_crud<TEntity>(std::forward<decltype(err)>(err), "create failed");
            });
    }

    Result<TEntity, CrudError> update(TEntity const& entity) {
        return get_transaction()
            .and_then([&entity](auto const& trx) {
                auto ser = rfl::Reflector<TEntity>::from_class(entity);

                return trx->connection().and_then(::sqlgen::write(std::move(ser)));
            })
            .and_then([&entity](auto&&) -> rfl::Result<TEntity> { return entity; })
            .transform_error([](auto&& err) {
                return map_crud<TEntity>(std::forward<decltype(err)>(err), "update failed");
            });
    }

    Result<bool, CrudError> delete_by_id(std::string id) {
        return get_transaction()
            .and_then([id](auto const& trx) {
                return trx->connection().and_then(::sqlgen::delete_from<SerializationType>
                                                  | ::sqlgen::where("id"_c == id));
            })
            .and_then([](auto&&) -> rfl::Result<bool> { return true; })
            .transform_error([id](auto&& err) {
                return map_crud<bool>(std::forward<decltype(err)>(err),
                                      "delete failed for id " + id);
            });
    }

private:
    [[nodiscard]] rfl::Result<std::shared_ptr<SqlgenUnitOfWork>> get_transaction() const {
        if (m_unit_of_work.expired()) {
            return rfl::error("Error");
        }
        return m_unit_of_work.lock();
    }

    std::weak_ptr<SqlgenUnitOfWork> m_unit_of_work;
};

// handy aliases
template<typename TEntity> using ReadWriteRepository = SqlgenRepository<TEntity>;
template<typename TEntity> using ReadOnlyRepository = SqlgenRepository<TEntity>;

} // namespace bxt::adapters::sqlgen
