/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/events/EventBase.h"
#include "core/domain/events/PackageEvents.h"

#include <dexode/EventBus.hpp>
#include <iostream>
#include <memory>

namespace bxt::Infrastructure {
class EventLogger {
public:
    EventLogger(std::shared_ptr<dexode::EventBus> evbus)
        : m_listener(dexode::EventBus::Listener::createNotOwning(*evbus)) {
    }

private:
    dexode::EventBus::Listener m_listener;
};
} // namespace bxt::Infrastructure
