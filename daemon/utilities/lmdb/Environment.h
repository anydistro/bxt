/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/locked.h"

#include <coro/io_scheduler.hpp>
#include <coro/mutex.hpp>
#include <coro/shared_mutex.hpp>
#include <coro/task.hpp>
#include <kangaru/autowire.hpp>
#include <lmdbxx/lmdb++.h>
#include <memory>

namespace bxt::Utilities::LMDB {
class Environment {
public:
    Environment(std::shared_ptr<coro::io_scheduler> scheduler)
        : m_env(lmdb::env::create()), m_mutex(scheduler) {}

    coro::task<std::unique_ptr<locked<lmdb::txn>>> begin_rw_txn() {
        auto result = std::make_unique<locked<lmdb::txn>>(
            co_await m_mutex.lock(), lmdb::txn::begin(m_env));

        co_return result;
    }

    coro::task<std::unique_ptr<locked<lmdb::txn>>> begin_ro_txn() {
        auto result = std::make_unique<locked<lmdb::txn>>(
            co_await m_mutex.lock_shared(),
            lmdb::txn::begin(m_env, nullptr, MDB_RDONLY));

        co_return result;
    }

    lmdb::env& env() { return m_env; }

private:
    lmdb::env m_env;
    coro::shared_mutex<coro::io_scheduler> m_mutex;
};

} // namespace bxt::Utilities::LMDB
