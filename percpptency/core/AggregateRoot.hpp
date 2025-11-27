/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "DomainEvent.hpp"

#include <vector>

namespace percpptency {

// CRTP mixin for entities that need domain event tracking
// Usage: class MyEntity : public AggregateRoot<MyEntity> { ... }
template<typename TDerived> class AggregateRoot {
public:
    AggregateRoot() = default;
    ~AggregateRoot() = default;
    
    AggregateRoot(AggregateRoot const&) = default;
    AggregateRoot& operator=(AggregateRoot const&) = default;
    AggregateRoot(AggregateRoot&&) = default;
    AggregateRoot& operator=(AggregateRoot&&) = default;

    [[nodiscard]] std::vector<DomainEvent> const&
        uncommitted_events() const {
        return m_uncommitted_events;
    }

    [[nodiscard]] std::vector<DomainEvent> take_uncommitted_events() {
        auto events = std::move(m_uncommitted_events);
        m_uncommitted_events.clear();
        return events;
    }

    void mark_events_as_committed() {
        m_uncommitted_events.clear();
    }

protected:
    void record_event(DomainEvent event) {
        m_uncommitted_events.push_back(std::move(event));
    }

private:
    std::vector<DomainEvent> m_uncommitted_events;
};

} // namespace percpptency
