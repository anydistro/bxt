/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <ranges>

#include "domain/User.hpp"
#include "utils/ConvertTo.hpp"
#include "UserMessages.hpp"

namespace bxt::utils {
// Specialization for User -> UserResponse
template<typename TTo> struct convert_to<TTo, Users::domain::User> {
    Result<TTo, ConversionError> operator()(Users::domain::User const& from) const {
        if constexpr (requires {
                          TTo {.name = std::string(), .permissions = std::set<std::string>()};
                      }) {
            return TTo {.name = from.name(),
                        .permissions =
                            from.permissions()
                            | std::views::transform(bxt::to_string<Users::domain::Permission>)
                            | std::ranges::to<std::set>()};
        } else if constexpr (requires {
                                 TTo {.name = std::string(),
                                      .password = std::optional<std::string>()};
                             }) {
            return TTo {.name = from.name(), .password = from.password()};
        } else if constexpr (requires { TTo {.name = std::string()}; }) {
            return TTo {.name = from.name()};
        } else {
            return cpperr::make_error(ConversionError::InvalidArgument);
        }
    }
};

template<typename TFrom> struct convert_to<Users::domain::User, TFrom> {
    Result<Users::domain::User, ConversionError> operator()(TFrom const& from) const {
        using namespace Users::domain;
        auto user = User::create(value_objects::Name::create(from.name).value(),
                                 []<typename T>(T const& req) {
                                     if constexpr (requires { req.password; }) {
                                         return req.password;
                                     } else {
                                         return std::optional<std::string> {};
                                     }
                                 }(from));

        if constexpr (requires { from.permissions; }) {
            if (from.permissions) {
                for (auto const& permission : *from.permissions) {
                    user->grant_permission(Permission::create(permission).value());
                }
            }
        }
        return *user;
    }
};
} // namespace bxt::utils
