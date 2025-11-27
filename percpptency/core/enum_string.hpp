/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string_view>
#include <type_traits>

#include <rfl.hpp>

namespace percpptency {

// Generic to_string for any enum using reflect-cpp's enum_to_string
template<typename E>
    requires std::is_enum_v<E>
constexpr std::string_view to_string(E value) noexcept {
    return rfl::enum_to_string(value);
}

} // namespace percpptency
