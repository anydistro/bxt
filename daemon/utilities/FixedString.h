/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <algorithm>

// Needed for string literals to be used as template parameters.
template<unsigned N> struct FixedString {
    char buf[N + 1] {};
    constexpr FixedString(char const* s) { std::copy(s, s + N, buf); }
    constexpr operator char const*() const { return buf; }
};

template<unsigned N> FixedString(char const (&)[N]) -> FixedString<N - 1>;
