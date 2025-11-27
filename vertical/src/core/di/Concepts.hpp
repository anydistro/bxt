/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <boost/di.hpp>
#include <concepts>
#include <type_traits>

namespace bxt {

template<typename T>
concept Module = requires(T t) {
    // A module must be default constructible
    std::is_default_constructible_v<T>;

    // A module must have a name
    { t.name() } -> std::convertible_to<std::string_view>;

    // A module must be able to initialize itself (can be a no-op)
    { t.initialize() } -> std::same_as<void>;
};

} // namespace bxt
