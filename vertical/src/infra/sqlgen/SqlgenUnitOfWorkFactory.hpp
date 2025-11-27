/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>

#include <fmt/format.h>
#include <sqlgen.hpp>
#include <sqlgen/begin_transaction.hpp>
#include <sqlgen/write.hpp>

#include <cpperr.hpp>
#include "infra/logging/Logging.hpp"
#include "core/domain/repository_errors.hpp"
#include "SqlgenRepository.hpp"
#include "SqlgenSettings.hpp"
#include "SqlgenUnitOfWork.hpp"

namespace bxt::adapters::sqlgen {
class SqlgenUnitOfWorkFactory {
    using TransactionError = domain::repo::TransactionError;

public:
    explicit SqlgenUnitOfWorkFactory(SqlgenSettings settings)
        : m_settings(std::move(settings)) {
    }

    Result<std::shared_ptr<SqlgenUnitOfWork>, TransactionError>
        create(SqlgenUnitOfWork::AutoCommitStrategy strategy) {
        auto connect_result = ::sqlgen::sqlite::connect("./bxt.db");
        if (!connect_result.has_value()) {
            bxt::logf("SqlgenUnitOfWorkFactory::create: Failed to connect to database: {}",
                      connect_result.error().what());
            return cpperr::make_error(TransactionError::BeginFailed,
                                      connect_result.error().what());
        }

        auto transaction_result = ::sqlgen::begin_transaction(std::move(*connect_result));
        if (!transaction_result) {
            return cpperr::make_error(TransactionError::BeginFailed,
                                      transaction_result.error().what());
        }

        auto connection = std::move(*transaction_result);
        return std::make_shared<SqlgenUnitOfWork>(std::move(connection), strategy);
    }

    template<typename... TEntity>
    Task<std::tuple<std::shared_ptr<SqlgenUnitOfWork>,
                    std::optional<cpperr::error<TransactionError>>,
                    SqlgenRepository<TEntity>...>>
        with_ro_repositories(
            SqlgenUnitOfWork::AutoCommitStrategy strategy = SqlgenUnitOfWork::ManualCommit) {
        auto uow_result = create(strategy);
        if (!uow_result) {
            co_return std::make_tuple(nullptr, std::make_optional(uow_result.error()),
                                      SqlgenRepository<TEntity>(nullptr)...);
        }
        auto uow_ptr = std::move(*uow_result);
        co_return std::make_tuple(uow_ptr, std::nullopt, SqlgenRepository<TEntity>(uow_ptr)...);
    }

    template<typename... TEntity>
    Task<std::tuple<std::shared_ptr<SqlgenUnitOfWork>,
                    std::optional<cpperr::error<TransactionError>>,
                    SqlgenRepository<TEntity>...>>
        with_rw_repositories(
            SqlgenUnitOfWork::AutoCommitStrategy strategy = SqlgenUnitOfWork::ManualCommit) {
        auto uow_result = create(strategy);
        if (!uow_result) {
            co_return std::make_tuple(nullptr, std::make_optional(uow_result.error()),
                                      SqlgenRepository<TEntity>(nullptr)...);
        }
        auto uow_ptr = std::move(*uow_result);
        co_return std::make_tuple(uow_ptr, std::nullopt, SqlgenRepository<TEntity>(uow_ptr)...);
    }

private:
    SqlgenSettings m_settings;
};
} // namespace bxt::adapters::sqlgen
