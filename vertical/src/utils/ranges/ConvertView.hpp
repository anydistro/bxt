/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <ranges>
#include <type_traits>
#include <utility>

#include "utils/ConvertTo.hpp"

namespace bxt::views {

template<typename TTargetType>
auto const convert = std::views::transform([](auto&& source) {
    return utils::convert_to<TTargetType, std::remove_cvref_t<decltype(source)>> {}(
        std::forward<decltype(source)>(source));
});

template<typename TTargetType>
auto const try_convert = std::views::transform([](auto&& source) {
    return utils::try_convert_to<TTargetType, std::remove_cvref_t<decltype(source)>>(
        std::forward<decltype(source)>(source));
});

} // namespace bxt::views
