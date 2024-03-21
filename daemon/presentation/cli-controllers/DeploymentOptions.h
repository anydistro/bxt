/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/configuration/Configuration.h"

#include <string>
namespace bxt::Presentation {
struct DeploymentOptions : public Utilities::Configurable {
    std::string key = "KEY0";

    void serialize(Utilities::Configuration& config) override {
        config.set("deployment-key", key);
    }

    void deserialize(const Utilities::Configuration& config) override {
        key = config.get<std::string>("deployment-key").value_or(key);
    }
};
} // namespace bxt::Presentation
