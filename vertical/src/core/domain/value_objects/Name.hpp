/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <optional>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <rfl/parsing/CustomParser.hpp>
#include <rfl/Result.hpp>

#include "infra/reflect/StringReflector.hpp"
#include "core/types/to_string.hpp"

namespace bxt::domain::value_objects {

class Name {
public:
    static std::optional<Name> create(std::string_view name_string) {
        if (name_string.empty()) {
            return std::nullopt;
        }
        return Name(name_string);
    }

    operator std::string const&() const {
        return m_value;
    }

    auto operator<=>(Name const& other) const = default;

private:
    explicit Name(std::string_view name_string)
        : m_value(name_string) {
    }

    std::string m_value;
};

} // namespace bxt::domain::value_objects

template<>
inline bxt::Result<bxt::domain::value_objects::Name, bxt::err::ParseError>
    bxt::from_string<bxt::domain::value_objects::Name>(std::string_view str) {
    auto name = bxt::domain::value_objects::Name::create(str);

    if (!name) {
        throw std::invalid_argument("Invalid name string");
    }
    return *name;
}

template<>
inline std::string bxt::to_string<bxt::domain::value_objects::Name>(
    bxt::domain::value_objects::Name const& value) {
    return {value};
}

template<> struct fmt::formatter<bxt::domain::value_objects::Name> : fmt::formatter<std::string> {
    template<typename FormatCtx>
    auto format(bxt::domain::value_objects::Name const& a, FormatCtx& ctx) const {
        return fmt::formatter<std::string>::format(bxt::to_string(a), ctx);
    }
};

BXT_REFLECT_DECLARE_STRING_REFLECTOR(bxt::domain::value_objects::Name)
