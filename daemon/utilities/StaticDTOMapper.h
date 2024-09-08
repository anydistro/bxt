/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <ranges>
#include <vector>

namespace bxt::Utilities {

template<typename TEntity, typename TDTO> struct StaticDTOMapper {
    static TDTO to_dto(TEntity const& from);
    static TEntity to_entity(const TDTO& from);
};

static constexpr auto map_entries = [](auto const& entries, auto const& Transformer) {
    return entries | std::views::transform(Transformer) | std::ranges::to<std::vector>();
};

} // namespace bxt::Utilities
