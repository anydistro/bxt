/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "LmdbEnvironment.hpp"
#include "shared/domain/Errors.hpp"
#include "shared/domain/UnitOfWorkBase.hpp"
#include "core/error/kinds.hpp"

namespace bxt::adapters::lmdb {

class LmdbUnitOfWork : public domain::UnitOfWorkBase<LmdbUnitOfWork> {
    using TransactionError = domain::TransactionError;

public:
    LmdbUnitOfWork(utils::SharedLocked<lmdbxx::txn> txn, auto di_container)
        : m_txn(std::move(txn))
        , m_di_container(std::move(di_container)) {
    }

    template<typename TIRepo> ResultTask<TIRepo, TransactionError> get_repository(bool read_write) {
        if (read_write && m_txn.& MDB_RDONLY) {
            co_return bxt::make_error<TransactionError>(
                earg("Cannot get write repository in read-only transaction"));
        }
        co_return TIRepo(m_txn);
    }

    VoidResultTask<TransactionError> commit() final {
        try {
            m_txn->commit();
            co_return {};
        } catch (lmdbxx::error const& e) {
            co_return bxt::make_error<TransactionError>(earg(e.what()));
        }
    }

    VoidResultTask<TransactionError> abort() final {
        try {
            m_txn->abort();
            co_return {};
        } catch (lmdbxx::error const& e) {
            co_return bxt::make_error<TransactionError>(earg(e.what()));
        }
    }

private:
    utils::SharedLocked<lmdbxx::txn> m_txn;
    std::shared_ptr<void> m_di_container;
};

} // namespace bxt::adapters::lmdb
