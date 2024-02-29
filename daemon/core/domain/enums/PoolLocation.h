/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once
#include "utilities/to_string.h"

#include <algorithm>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <optional>

namespace bxt::Core::Domain {
enum class PoolLocation { Overlay, Automated, Sync, Unknown };
constexpr static frozen::unordered_map<PoolLocation, frozen::string, 3>
    pool_location_names = {{PoolLocation::Sync, "sync"},
                           {PoolLocation::Overlay, "overlay"},
                           {PoolLocation::Automated, "automated"}};

template<typename TMap>
std::optional<PoolLocation> select_preferred_pool_location(const TMap& map) {
    static_assert(
        std::same_as<typename TMap::key_type, Core::Domain::PoolLocation>,
        "Key should be a PoolLocation type");
    if (map.empty()) { return {}; }

    return std::ranges::min_element(
               map, {},
               [](const auto& el) { return static_cast<int>(el.first); })
        ->first;
}

} // namespace bxt::Core::Domain

namespace bxt {
template<> std::string to_string(const Core::Domain::PoolLocation& location);
} // namespace bxt
