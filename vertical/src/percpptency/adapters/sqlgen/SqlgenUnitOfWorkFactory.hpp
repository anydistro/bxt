/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>

#include <cpperr/Error.hpp>
#include <cpperr/Factory.hpp>
#include <sqlgen/begin_transaction.hpp>
#include <sqlgen/sqlite/connect.hpp>

#include "../../core/UnitOfWork.hpp"
#include "SqlgenRepository.hpp"
#include "SqlgenUnitOfWork.hpp"

namespace percpptency::adapters::sqlgen {

class SqlgenUnitOfWorkFactory {
public:
    explicit SqlgenUnitOfWorkFactory(std::string db_path = "./app.db")
        : m_db_path(std::move(db_path)) {
    }

    cpperr::result<std::shared_ptr<SqlgenUnitOfWork>, TransactionError>
        create(SqlgenUnitOfWork::AutoCommitStrategy strategy = 
               SqlgenUnitOfWork::ManualCommit) {
        auto connect_result = ::sqlgen::sqlite::connect(m_db_path);
        if (!connect_result.has_value()) {
            return cpperr::make_error(TransactionError::BeginFailed,
                                      connect_result.error().what());
        }

        auto transaction_result = ::sqlgen::begin_transaction(std::move(*connect_result));
        if (!transaction_result) {
            return cpperr::make_error(TransactionError::BeginFailed,
                                      transaction_result.error().what());
        }

        return std::make_shared<SqlgenUnitOfWork>(std::move(*transaction_result), strategy);
    }

    template<typename... TEntity>
    Task<std::tuple<std::shared_ptr<SqlgenUnitOfWork>,
                    std::optional<cpperr::error<TransactionError>>,
                    SqlgenRepository<TEntity>...>>
        with_ro_repositories(SqlgenUnitOfWork::AutoCommitStrategy strategy = 
                             SqlgenUnitOfWork::ManualCommit) {
        auto uow_result = create(strategy);
        if (!uow_result) {
            co_return std::make_tuple(nullptr, std::make_optional(uow_result.error()),
                                      SqlgenRepository<TEntity>(nullptr)...);
        }
        auto uow_ptr = std::move(*uow_result);
        co_return std::make_tuple(uow_ptr, std::nullopt, 
                                  SqlgenRepository<TEntity>(uow_ptr)...);
    }

    template<typename... TEntity>
    Task<std::tuple<std::shared_ptr<SqlgenUnitOfWork>,
                    std::optional<cpperr::error<TransactionError>>,
                    SqlgenRepository<TEntity>...>>
        with_rw_repositories(SqlgenUnitOfWork::AutoCommitStrategy strategy = 
                             SqlgenUnitOfWork::ManualCommit) {
        auto uow_result = create(strategy);
        if (!uow_result) {
            co_return std::make_tuple(nullptr, std::make_optional(uow_result.error()),
                                      SqlgenRepository<TEntity>(nullptr)...);
        }
        auto uow_ptr = std::move(*uow_result);
        co_return std::make_tuple(uow_ptr, std::nullopt,
                                  SqlgenRepository<TEntity>(uow_ptr)...);
    }

private:
    std::string m_db_path;
};

} // namespace percpptency::adapters::sqlgen
