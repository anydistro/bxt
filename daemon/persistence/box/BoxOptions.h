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

struct BoxOptions {
    std::filesystem::path box_path = "box";

    void serialize(Utilities::Configuration &config) {
        config.set("box-path", box_path.string());
    }
    void deserialize(const Utilities::Configuration &config) {
        box_path = config.get<std::string>("box-path").value_or(box_path);
    }
};

} // namespace bxt::Persistence::Box
