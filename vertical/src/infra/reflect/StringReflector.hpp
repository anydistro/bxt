/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <expected>
#include <string>

#include <rfl/Result.hpp>

#include "infra/sqlgen/SqlgenHeaders.hpp"
#include "core/types/to_string.hpp"

namespace bxt::reflect {

/**
 * @brief Base reflector for string-convertible types
 *
 * @tparam T The type to reflect
 */
template<typename T> struct StringReflectorBase {
    using Type = T;
    using ReflType = std::string;

    static rfl::Result<Type> to(ReflType const& v) noexcept {
        auto result = bxt::from_string<Type>(v);
        if (result) {
            return *result;
        } else {
            return std::unexpected(rfl::Error(result.error().what()));
        }
    }

    static ReflType from(Type const& v) noexcept {
        return bxt::to_string(v);
    }

    static sqlgen::dynamic::Type to_type() noexcept {
        return sqlgen::dynamic::types::Text {};
    }
};

} // namespace bxt::reflect

/**
 * @brief Macro for declaring reflectors
 */
#define BXT_REFLECT_DECLARE_STRING_REFLECTOR(Type)                                             \
    namespace rfl {                                                                            \
        template<> struct Reflector<Type> : public bxt::reflect::StringReflectorBase<Type> {}; \
    }
