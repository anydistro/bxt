/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string>
namespace bxt::adapters::sqlgen {

struct SqlgenSettings {
    std::string connection_string = "./bxt.db";
};

}; // namespace bxt::adapters::sqlgen
