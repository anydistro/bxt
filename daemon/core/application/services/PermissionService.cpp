/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PermissionService.h"

#include <algorithm>

namespace bxt::Core::Application {

coro::task<bool> PermissionService::check(std::string_view const target_permission,
                                          std::string const user_name) {
    return check(std::vector<std::string_view> {target_permission}, user_name);
}

coro::task<bool> PermissionService::check(std::vector<std::string_view> const target_permissions,
                                          std::string const user_name) {
    auto user = co_await m_repository.find_by_id_async(user_name, co_await m_uow_factory());

    if (!user) {
        co_return {};
    }

    for (auto const& target_permission : target_permissions) {
        if (!user->has_permission(target_permission)) {
            co_return false;
        }
    }

    co_return true;
}

} // namespace bxt::Core::Application
