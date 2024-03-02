/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/events/IntegrationEventBase.h"
#include "core/domain/entities/Package.h"

#include <fmt/format.h>
#include <vector>

namespace bxt::Core::Application::Events {

struct SyncStarted : public IntegrationEventBase {
    SyncStarted() = default;

    virtual std::string message() const { return "Sync started"; }
};

struct SyncFinished : public IntegrationEventBase {
    SyncFinished() = default;
    SyncFinished(std::vector<bxt::Core::Domain::Package>&& packages)
        : packages_synced(std::move(packages)) {}

    std::vector<bxt::Core::Domain::Package> packages_synced;

    virtual std::string message() const { return "Sync finished"; }
};

} // namespace bxt::Core::Application::Events
