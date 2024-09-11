/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PoolLocation.h"

using bxt::Core::Domain::pool_location_names;
using bxt::Core::Domain::PoolLocation;

template<> std::string bxt::to_string(Core::Domain::PoolLocation const& location) {
    return {pool_location_names.at(location).begin(), pool_location_names.at(location).end()};
}
