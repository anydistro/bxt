/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/repositories/UnitOfWorkBase.h"
#include "utilities/lmdb/Environment.h"
#include "utilities/locked.h"

#include <coro/task.hpp>
#include <memory>
namespace bxt::Persistence {

class LmdbUnitOfWork : public Core::Domain::UnitOfWorkBase {
public:
    explicit LmdbUnitOfWork(std::shared_ptr<Utilities::LMDB::Environment> env)
        : m_env(std::move(env)) {}

    virtual ~LmdbUnitOfWork() = default;

    coro::task<Result<void>> commit_async() override {
        m_txn->value.commit();
        co_return {};
    }

    coro::task<Result<void>> rollback_async() override {
        m_txn->value.abort();
        co_return {};
    }

    coro::task<Result<void>> begin_async() override {
        m_txn = co_await m_env->begin_rw_txn();
        co_return {};
    }

    coro::task<Result<void>> begin_ro_async() override {
        m_txn = co_await m_env->begin_ro_txn();
        co_return {};
    }

    Utilities::locked<lmdb::txn>& txn() const { return *m_txn; }

private:
    std::shared_ptr<Utilities::LMDB::Environment> m_env;
    std::unique_ptr<Utilities::locked<lmdb::txn>> m_txn;
};

struct LmdbUnitOfWorkFactory : public Core::Domain::UnitOfWorkBaseFactory {
    explicit LmdbUnitOfWorkFactory(
        std::shared_ptr<Utilities::LMDB::Environment> env)
        : m_env(std::move(env)) {}

    coro::task<std::shared_ptr<Core::Domain::UnitOfWorkBase>>
        operator()(bool rw = false) override {
        auto uow = std::make_shared<LmdbUnitOfWork>(m_env);
        if (rw) {
            co_await uow->begin_async();
        } else {
            co_await uow->begin_ro_async();
        }
        co_return uow;
    }

private:
    std::shared_ptr<Utilities::LMDB::Environment> m_env;
};
} // namespace bxt::Persistence
