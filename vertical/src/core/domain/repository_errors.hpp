/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <cpperr/Error.hpp>
#include <string_view>

namespace bxt::domain::repo {

enum class TransactionError {
    BeginFailed,
    CommitFailed,
    RollbackFailed,
    ConcurrencyConflict,
    Timeout,
    LockAcquisitionFailed,
    ConnectionError,
    InvalidState
};

inline std::string_view to_string(TransactionError e) {
    switch (e) {
        case TransactionError::BeginFailed: return "BeginFailed";
        case TransactionError::CommitFailed: return "CommitFailed";
        case TransactionError::RollbackFailed: return "RollbackFailed";
        case TransactionError::ConcurrencyConflict: return "ConcurrencyConflict";
        case TransactionError::Timeout: return "Timeout";
        case TransactionError::LockAcquisitionFailed: return "LockAcquisitionFailed";
        case TransactionError::ConnectionError: return "ConnectionError";
        case TransactionError::InvalidState: return "InvalidState";
    }
    return "Unknown";
}

} // namespace bxt::domain::repo
