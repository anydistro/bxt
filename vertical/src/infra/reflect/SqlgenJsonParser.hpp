/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <type_traits>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen/dynamic/types.hpp>
#include <sqlgen/parsing/Parser_default.hpp>

namespace sqlgen::parsing {

/**
 * @brief Concept to detect container types that should be serialized as JSON
 */
template<typename T>
concept IsJsonSerializable = requires(T t, std::string s) {
    { rfl::json::write(t) } -> std::same_as<std::string>;
    { rfl::json::read<T>(std::string_view(s)) } -> std::same_as<rfl::Result<T>>;
    { t.begin() };
    { t.end() };
};
/**
 * @brief Specialization of sqlgen Parser for container types and reflectable types
 *
 * This allows container types and reflectable types to be stored as JSON strings
 * in the database using rfl::json serialization.
 */
template<IsJsonSerializable T> struct Parser<T> {
    using Type = std::remove_cvref_t<T>;

    static Result<Type> read(std::optional<std::string> const& _str) noexcept {
        try {
            if (!_str.has_value() || _str->empty()) {
                return error("Cannot parse empty JSON string");
            }

            auto result = rfl::json::read<Type>(*_str);
            if (!result) {
                return error("Failed to parse JSON: " + result.error().what());
            }

            return result.value();
        } catch (std::exception const& e) {
            return error(std::string("JSON parsing error: ") + e.what());
        }
    }

    static std::optional<std::string> write(Type const& _t) noexcept {
        try {
            return rfl::json::write(_t);
        } catch (std::exception const& e) {
            return std::nullopt;
        }
    }

    static dynamic::Type to_type() noexcept {
        return dynamic::types::Text {};
    }
};

} // namespace sqlgen::parsing
