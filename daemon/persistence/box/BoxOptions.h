/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/configuration/Configuration.h"

#include <filesystem>

namespace bxt::Persistence::Box {

struct BoxOptions : public bxt::Utilities::Configurable {
    std::filesystem::path box_path = "box";

    virtual void serialize(Utilities::Configuration &config) override {
        config.set("box-path", box_path.string());
    }
    virtual void deserialize(const Utilities::Configuration &config) override {
        box_path = config.get<std::string>("box-path").value_or(box_path);
    }
};

} // namespace bxt::Persistence::Box
