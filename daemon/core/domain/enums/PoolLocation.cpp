/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "PoolLocation.h"

using bxt::Core::Domain::pool_location_names;
using bxt::Core::Domain::PoolLocation;

template<>
std::string bxt::to_string(const Core::Domain::PoolLocation& location) {
    return {pool_location_names.at(location).begin(),
            pool_location_names.at(location).end()};
}
