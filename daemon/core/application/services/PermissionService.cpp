/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PermissionService.h"

#include <algorithm>

namespace bxt::Core::Application {

coro::task<bool>
    PermissionService::check(const std::string_view target_permission,
                             const std::string user_name) {
    return check(std::vector<std::string_view> {target_permission}, user_name);
}

coro::task<bool> PermissionService::check(
    const std::vector<std::string_view> target_permissions,
    const std::string user_name) {
    auto user = co_await m_repository.find_by_id_async(user_name);

    if (!user) { co_return {}; }

    for (const auto& target_permission : target_permissions) {
        if (!user->has_permission(target_permission)) { co_return false; }
    }

    co_return true;
}

} // namespace bxt::Core::Application
