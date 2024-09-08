/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/events/IntegrationEventBase.h"
#include "core/domain/entities/Package.h"

#include <fmt/format.h>
#include <string>

namespace bxt::Core::Application::Events {

struct DeploySuccess : public IntegrationEventBase {
    DeploySuccess() = default;
    DeploySuccess(std::string deployment_url, std::vector<bxt::Core::Domain::Package>&& packages)
        : deployment_url(std::move(deployment_url))
        , added_packages(std::move(packages)) {
    }

    std::string deployment_url;
    std::vector<bxt::Core::Domain::Package> added_packages;

    std::string message() const override {
        return fmt::format("Deployment successful at {}: {} packages added", deployment_url,
                           added_packages.size());
    }
};
} // namespace bxt::Core::Application::Events
