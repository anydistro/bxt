/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "events.h"
#include "utilities/log/Logging.h"

#include <coro/task.hpp>
#include <dexode/EventBus.hpp>
#include <memory>
#include <typeindex>

namespace bxt::Utilities {

class EventBusDispatcher {
public:
    EventBusDispatcher(std::shared_ptr<dexode::EventBus> evbus)
        : m_evbus(evbus) {
    }

    template<typename TEvent> inline void process(TEvent eptr) {
        if (auto const it = bxt::events::event_map.find(std::type_index(typeid(*eptr)));
            it != bxt::events::event_map.cend()) {
            auto tindex = std::type_index(typeid(*eptr));
            auto str = tindex.name();
            it->second(eptr.get(), m_evbus);
        }
    }

    template<typename TList> coro::task<void> dispatch_async(TList evlist) {
        if (!m_evbus)
            co_return;

        for (auto const& event : evlist) {
            process(event);

            logt(event->message());
        }
    }

    template<typename TEventBase> coro::task<void> dispatch_single_async(TEventBase event) {
        if (!m_evbus)
            co_return;

        process(event);

        logt(event->message());
    }

private:
    std::shared_ptr<dexode::EventBus> m_evbus;
};

} // namespace bxt::Utilities
