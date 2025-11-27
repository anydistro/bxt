/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <rfl.hpp>
#include <sqlgen/dynamic/types.hpp>
#include <sqlgen/parsing/Parser_default.hpp>

namespace sqlgen::parsing {

/**
 * @brief Concept to detect types with rfl::Reflector specializations
 */
template<typename T>
concept HasReflector = requires {
    typename rfl::Reflector<T>;
    typename rfl::Reflector<T>::ReflType;
    {
        rfl::Reflector<T>::to(std::declval<typename rfl::Reflector<T>::ReflType>())
    } -> std::same_as<rfl::Result<T>>;
    {
        rfl::Reflector<T>::from(std::declval<T>())
    } -> std::same_as<typename rfl::Reflector<T>::ReflType>;
    { rfl::Reflector<T>::to_type() } -> std::same_as<dynamic::Type>;
};

/**
 * @brief Specialization of sqlgen Parser for types with rfl::Reflector
 *
 * This allows types with Reflector specializations to be used with sqlgen
 * by delegating to their ReflType parser.
 */
template<HasReflector T> struct Parser<T> {
    using Type = std::remove_cvref_t<T>;
    using ReflectorType = rfl::Reflector<Type>;
    using ReflType = typename ReflectorType::ReflType;

    static Result<Type> read(std::optional<std::string> const& _str) noexcept {
        try {
            auto refl_result = Parser<ReflType>::read(_str);
            if (!refl_result) {
                return error(refl_result.error());
            }

            return ReflectorType::to(refl_result.value());
        } catch (std::exception const& e) {
            return error(e.what());
        }
    }

    static std::optional<std::string> write(Type const& _t) noexcept {
        try {
            auto refl_data = ReflectorType::from(_t);
            return Parser<ReflType>::write(refl_data);
        } catch (std::exception const& e) {
            return std::nullopt;
        }
    }

    static dynamic::Type to_type() noexcept {
        return ReflectorType::to_type();
    }
};

} // namespace sqlgen::parsing
