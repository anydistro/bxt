/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/events/IntegrationEventBase.h"
#include "core/domain/entities/Package.h"
#include "core/domain/events/PackageEvents.h"

#include <fmt/format.h>
#include <vector>

namespace bxt::Core::Application::Events {

struct SyncStarted : public IntegrationEventBase {
    SyncStarted() = default;

    std::string user_name;

    std::string message() const override {
        return fmt::format("Sync started by {}", user_name);
    }
};

struct SyncFinished : public IntegrationEventBase {
    SyncFinished() = default;
    SyncFinished(std::vector<Core::Domain::Package>&& packages,
                 std::vector<Core::Domain::Package::TId>&& deleted_package_ids,
                 std::string user_name)
        : added_packages(std::move(packages))
        , deleted_package_ids(std::move(deleted_package_ids))
        , user_name(std::move(user_name)) {
    }

    std::vector<Core::Domain::Package> added_packages;

    std::vector<Core::Domain::Package::TId> deleted_package_ids;

    std::string user_name;

    std::string message() const override {
        return fmt::format("Sync started by {} was finished: {} packages "
                           "added, {} packages deleted",
                           user_name, added_packages.size(), deleted_package_ids.size());
    }
};

} // namespace bxt::Core::Application::Events
