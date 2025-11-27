/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <cctype>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#include <fmt/format.h>
#include <rfl/parsing/CustomParser.hpp>

#include "infra/reflect/StringReflector.hpp"
#include "core/result_types.hpp"
#include "core/types/Path.hpp"
#include "core/types/TimePoint.hpp"
#include "core/types/to_string.hpp"
#include "core/domain/value_objects/Name.hpp"
#include "core/error/kinds.hpp"
#include "core/error/kinds.hpp"
#include "utils/ranges/TryTo.hpp"

namespace bxt::Users::domain {

using namespace ::bxt::domain;

class Permission {
public:
    static Result<Permission, err::ParseError> create(std::string_view permission_string) {
        if (permission_string.empty()) {
            return cpperr::make_error(err::ParseError::InvalidValue,
                                      "Permission string is empty");
        }
        auto result = permission_string | std::views::split('.')
                      | std::views::filter([](auto const& tag) { return !tag.empty(); })
                      | std::views::transform([](auto const& tag) {
                            return bxt::from_string<value_objects::Name>(std::string_view(tag));
                        })
                      | bxt::ranges::try_to<std::vector>;

        if (!result.has_value()) {
            return std::unexpected {std::move(result.error())};
        }

        return Permission {std::move(*result)};
    }
    std::vector<value_objects::Name> const& tags() const {
        return m_permission_tags;
    }

    auto operator<=>(Permission const& other) const = default;

private:
    explicit Permission(std::vector<value_objects::Name> permission_tags)
        : m_permission_tags(std::move(permission_tags)) {
    }

    std::vector<value_objects::Name> m_permission_tags;
};

} // namespace bxt::Users::domain

template<> inline std::string bxt::to_string(bxt::Users::domain::Permission const& permission) {
    return fmt::format(
        "{}", fmt::join(permission.tags()
                            | std::views::transform(bxt::to_string<domain::value_objects::Name>),
                        "."));
}
template<>
inline bxt::Result<bxt::Users::domain::Permission, bxt::err::ParseError>
    bxt::from_string(std::string_view str) {
    return bxt::Users::domain::Permission::create(str);
}

BXT_REFLECT_DECLARE_STRING_REFLECTOR(bxt::Users::domain::Permission)
