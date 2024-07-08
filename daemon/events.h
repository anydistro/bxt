/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/events/CommitEvent.h"
#include "core/application/events/DeployEvent.h"
#include "core/application/events/IntegrationEventBase.h"
#include "core/application/events/SyncEvent.h"
#include "core/domain/events/EventBase.h"
#include "core/domain/events/PackageEvents.h"

#include <any>
#include <dexode/EventBus.hpp>
#include <functional>
#include <typeindex>

namespace bxt::events {

using namespace bxt::Core::Domain::Events;
using namespace bxt::Core::Application::Events;

using eventbus_visitor =
    std::function<void(std::any, std::shared_ptr<dexode::EventBus>&)>;

template<typename TEvent, typename TEventBase>
std::pair<std::type_index, eventbus_visitor> to_eventbus_visitor() {
    return {std::type_index(typeid(TEvent)),
            [](std::any event, std::shared_ptr<dexode::EventBus> evbus) {
                auto event_base = std::any_cast<TEventBase*>(event);
                evbus->postpone<TEvent>(*(dynamic_cast<TEvent*>(event_base)));
            }};
}

static inline std::unordered_map<std::type_index, eventbus_visitor> event_map {
    to_eventbus_visitor<PackageAdded, EventBase>(),
    to_eventbus_visitor<PackageRemoved, EventBase>(),
    to_eventbus_visitor<PackageUpdated, EventBase>(),
    to_eventbus_visitor<SyncStarted, IntegrationEventBase>(),
    to_eventbus_visitor<SyncFinished, IntegrationEventBase>(),
    to_eventbus_visitor<Commited, IntegrationEventBase>(),
    to_eventbus_visitor<DeploySuccess, IntegrationEventBase>(),

};

} // namespace bxt::events
