/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include <cstddef>

namespace bxt::MemoryLiterals {

constexpr std::size_t BinaryBase = 1024UL;
constexpr std::size_t DecimalBase = 1000UL;

constexpr std::size_t operator"" _KiB(unsigned long long size) {
    return size * BinaryBase;
}

constexpr std::size_t operator"" _MiB(unsigned long long size) {
    return size * BinaryBase * BinaryBase;
}

constexpr std::size_t operator"" _GiB(unsigned long long size) {
    return size * BinaryBase * BinaryBase * BinaryBase;
}

constexpr std::size_t operator"" _TiB(unsigned long long size) {
    return size * BinaryBase * BinaryBase * BinaryBase * BinaryBase;
}

constexpr std::size_t operator"" _PiB(unsigned long long size) {
    return size * BinaryBase * BinaryBase * BinaryBase * BinaryBase * BinaryBase;
}

constexpr std::size_t operator"" _KB(unsigned long long size) {
    return size * DecimalBase;
}

constexpr std::size_t operator"" _MB(unsigned long long size) {
    return size * DecimalBase * DecimalBase;
}

constexpr std::size_t operator"" _GB(unsigned long long size) {
    return size * DecimalBase * DecimalBase * DecimalBase;
}

constexpr std::size_t operator"" _TB(unsigned long long size) {
    return size * DecimalBase * DecimalBase * DecimalBase * DecimalBase;
}

constexpr std::size_t operator"" _PB(unsigned long long size) {
    return size * DecimalBase * DecimalBase * DecimalBase * DecimalBase * DecimalBase;
}

} // namespace bxt::MemoryLiterals
