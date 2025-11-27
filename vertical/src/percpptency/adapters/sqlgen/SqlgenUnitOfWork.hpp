/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <memory>

#include <coro/task.hpp>
#include <rfl/Result.hpp>
#include <sqlgen/begin_transaction.hpp>
#include <sqlgen/commit.hpp>
#include <sqlgen/sqlite/Connection.hpp>
#include <sqlgen/Transaction.hpp>

#include "../../core/UnitOfWork.hpp"

namespace percpptency::adapters::sqlgen {

using TransactionType = ::sqlgen::Transaction<::sqlgen::sqlite::Connection>;

class SqlgenUnitOfWork : public UnitOfWork,
                          public std::enable_shared_from_this<SqlgenUnitOfWork> {
public:
    enum AutoCommitStrategy { AutoCommit, ManualCommit };

    explicit SqlgenUnitOfWork(rfl::Result<rfl::Ref<TransactionType>>&& connection,
                              AutoCommitStrategy strategy = ManualCommit)
        : m_connection(std::move(connection))
        , m_strategy(strategy) {
    }

    ~SqlgenUnitOfWork() override {
        if (m_strategy == AutoCommit && m_connection) {
            (*m_connection)->commit();
        }
    }

    Task<void> commit() override {
        m_connection.and_then(::sqlgen::commit);
        co_return;
    }

    Task<void> rollback() override {
        // SQLite rollback happens automatically on transaction destruction
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

} // namespace percpptency::adapters::sqlgen
