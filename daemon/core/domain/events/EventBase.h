/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <chrono>
#include <fmt/format.h>
#include <memory>
namespace bxt::Core::Domain::Events {
struct EventBase {
    virtual ~EventBase() = default;

    std::chrono::time_point<std::chrono::system_clock> when = std::chrono::system_clock::now();

    virtual std::string message() const = 0;
};

using EventPtr = std::shared_ptr<EventBase>;

using EmptyEvent = EventBase;

} // namespace bxt::Core::Domain::Events
