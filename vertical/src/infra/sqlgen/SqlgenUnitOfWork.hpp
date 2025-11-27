/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <memory>
#include <utility>

#include <coro/task.hpp>
#include <rfl/Result.hpp>
#include <sqlgen/begin_transaction.hpp>
#include <sqlgen/sqlite/connect.hpp>
#include <sqlgen/sqlite/Connection.hpp>
#include <sqlgen/Transaction.hpp>

#include "core/domain/repository_errors.hpp"

namespace bxt::adapters::sqlgen {

using TransactionType = ::sqlgen::Transaction<::sqlgen::sqlite::Connection>;

class SqlgenUnitOfWork : std::enable_shared_from_this<SqlgenUnitOfWork> {
public:
    enum AutoCommitStrategy { AutoCommit, ManualCommit };

    explicit SqlgenUnitOfWork(rfl::Result<rfl::Ref<TransactionType>>&& connection,
                              AutoCommitStrategy strategy = ManualCommit)
        : m_connection(std::move(connection))
        , m_strategy(strategy) {
    }

    ~SqlgenUnitOfWork() {
        if (m_strategy == AutoCommit) {
            (*m_connection)->commit();
        }
    }

    Task<void> commit() {
        m_connection.and_then(::sqlgen::commit);
        co_return;
    }

    Task<void> rollback() {
        co_return;
    }

    rfl::Result<rfl::Ref<TransactionType>>& connection() {
        return m_connection;
    }

private:
    rfl::Result<rfl::Ref<TransactionType>> m_connection;
    AutoCommitStrategy m_strategy;
};

using SqlgenUnitOfWorkPtr = std::shared_ptr<SqlgenUnitOfWork>;
} // namespace bxt::adapters::sqlgen
