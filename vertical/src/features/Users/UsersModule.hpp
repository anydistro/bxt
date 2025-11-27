/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string_view>
#include <tuple>

#include <boost/di.hpp>

#include "UserController.hpp"

namespace bxt::Users {

class UsersModule {
public:
    // Define the tuple of controller types this module provides
    using controller_types = std::tuple<UserController>;

    UsersModule() = default;

    static std::string_view name() {
        return "Users";
    }

    auto configure([[maybe_unused]] auto&& injector) {
        namespace di = boost::di;
        // Configure module-specific dependencies
        return di::make_injector(std::move(injector));
    }

    void initialize() {
    }

    std::shared_ptr<UsersModule> operator()() const {
        return std::make_shared<UsersModule>();
    }
};

} // namespace bxt::Users
