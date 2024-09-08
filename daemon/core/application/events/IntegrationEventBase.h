/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <chrono>
#include <memory>
namespace bxt::Core::Application::Events {
struct IntegrationEventBase {
    virtual ~IntegrationEventBase() = default;

    std::chrono::time_point<std::chrono::system_clock> when = std::chrono::system_clock::now();

    virtual std::string message() const = 0;
};

using IntegrationEventPtr = std::shared_ptr<IntegrationEventBase>;

} // namespace bxt::Core::Application::Events
