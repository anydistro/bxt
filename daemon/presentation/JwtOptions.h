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

struct JwtOptions {
    std::string secret = "secret";
    std::string issuer = "bxt";

    void serialize(Utilities::Configuration& config) {
        config.set("jwt-secret", secret);
        config.set("jwt-issuer", issuer);
    }
    void deserialize(Utilities::Configuration const& config) {
        secret = config.get<std::string>("jwt-secret").value_or(secret);
        issuer = config.get<std::string>("jwt-issuer").value_or(issuer);
    }
};
} // namespace bxt::Presentation
