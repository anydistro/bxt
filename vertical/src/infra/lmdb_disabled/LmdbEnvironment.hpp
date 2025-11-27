/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <filesystem>
#include <memory>

#include <coro/io_scheduler.hpp>
#include <coro/shared_mutex.hpp>
#include <lmdbxx/lmdb++.h>

#include "shared/common.hpp"
#include "utils/Locked.hpp"
#include "utils/MemoryLiterals.hpp"

namespace bxt::adapters::lmdb {
using namespace MemoryLiterals;
namespace lmdbxx = ::lmdb;
class Environment {
    constexpr static size_t MaxReaders = 126;
    constexpr static size_t MaxDbs = 128;
    constexpr static size_t MapSize = 10_MiB;  // 10MB default
    constexpr static unsigned int Mode = 0644; // Default file permissions
    constexpr static int Flags = 0;            // Environment flags

public:
    struct Options {
        size_t max_readers;
        size_t max_dbs;
        size_t map_size;
        unsigned int mode;
        int flags;
    };

    Environment(std::shared_ptr<coro::io_scheduler> scheduler,
                std::filesystem::path const& path,
                Options const& options = Options {.max_readers = MaxReaders,
                                                  .max_dbs = MaxDbs,
                                                  .map_size = MapSize,
                                                  .mode = Mode,
                                                  .flags = Flags})
        : m_env(lmdbxx::env::create())
        , m_mutex(scheduler) {
        m_env.set_mapsize(options.map_size);
        m_env.set_max_readers(options.max_readers);
        m_env.set_max_dbs(options.max_dbs);
        m_env.open(path.c_str(), options.flags, options.mode);
    }

    Task<utils::SharedLocked<lmdbxx::txn>> begin_rw_txn() {
        co_return utils::SharedLocked<lmdbxx::txn>(co_await m_mutex.lock(), &lmdbxx::txn::abort,
                                                   lmdbxx::txn::begin(m_env));
    }
    Task<utils::SharedLocked<lmdbxx::txn>> begin_ro_txn() {
        co_return utils::SharedLocked<lmdbxx::txn>(co_await m_mutex.lock_shared(),
                                                   &lmdbxx::txn::abort,
                                                   lmdbxx::txn::begin(m_env, nullptr, MDB_RDONLY));
    }

    lmdbxx::env& env() {
        return m_env;
    }

private:
    lmdbxx::env m_env;
    coro::shared_mutex<coro::io_scheduler> m_mutex;
};

} // namespace bxt::adapters::lmdb
