/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include <coro/task.hpp>

#include "DomainEvent.hpp"
#include "enum_string.hpp"

namespace percpptency {

// Transaction lifecycle errors
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

// to_string provided by enum_string.hpp generic template

// Unit of Work interface - adapters implement this
class UnitOfWork {
public:
    virtual ~UnitOfWork() = default;

    virtual coro::task<void> commit() = 0;
    virtual coro::task<void> rollback() = 0;

    void add_events(std::vector<DomainEvent>&& events) {
        m_uncommitted_events.insert(m_uncommitted_events.end(),
                                    std::make_move_iterator(events.begin()),
                                    std::make_move_iterator(events.end()));
    }

    [[nodiscard]] std::vector<DomainEvent> const& uncommitted_events() const {
        return m_uncommitted_events;
    }

    [[nodiscard]] std::vector<DomainEvent> take_uncommitted_events() {
        auto events = std::move(m_uncommitted_events);
        m_uncommitted_events.clear();
        return events;
    }

protected:
    std::vector<DomainEvent> m_uncommitted_events;
};

// Alias matching Drogon's convention
template<typename T> using Task = coro::task<T>;

} // namespace percpptency
