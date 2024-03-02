/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <compare>
#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace bxt::Core::Domain {

class Name {
public:
    Name(const std::string& name_string) : m_value(name_string) {
        if (name_string.empty()) {
            throw new std::invalid_argument("Name cannot be empty");
        }
    }

    operator const std::string&() const { return m_value; }

    auto operator<=>(const Name& other) const = default;

private:
    std::string m_value;
};

} // namespace bxt::Core::Domain

template<>
struct fmt::formatter<bxt::Core::Domain::Name> : fmt::formatter<std::string> {
    template<typename FormatCtx>
    auto format(const bxt::Core::Domain::Name& a, FormatCtx& ctx) const {
        return fmt::formatter<std::string>::format(std::string(a), ctx);
    }
};
