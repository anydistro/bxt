/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/configuration/Configuration.h"

namespace bxt::Persistence::Box {

struct BoxOptions : public bxt::Utilities::Configurable {
    std::string location = "box/";

    virtual void serialize(Utilities::Configuration &config) override {
        config.set("location", location);
    }
    virtual void deserialize(const Utilities::Configuration &config) override {
        location = config.get<std::string>("location").value_or(location);
    }
};

} // namespace bxt::Persistence::Box
