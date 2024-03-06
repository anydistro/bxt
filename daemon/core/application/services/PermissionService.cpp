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

    std::vector<bool> matched_targets(target_permissions.size(), false);
    for (int i = 0; i < target_permissions.size(); i++) {
        matched_targets[i] = user->has_permission(target_permissions[i]);
    }

    co_return std::ranges::all_of(matched_targets, [](bool v) { return v; });
}

} // namespace bxt::Core::Application
