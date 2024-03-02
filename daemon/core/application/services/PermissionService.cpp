/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PermissionService.h"

#include <algorithm>

namespace bxt::Core::Application {

coro::task<PermissionService::Result<void>>
    PermissionService::add(const std::string user_name,
                           const std::string permission) {
    auto user = co_await m_repository.find_by_id_async(user_name);

    if (!user.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(user.error()), CrudError::ErrorType::EntityNotFound);
    }

    auto perms = user->permissions();

    perms.emplace(permission);

    user->set_permissions(perms);

    auto task = m_repository.update_async(std::vector {*user});

    auto result = co_await task;

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(result.error()), CrudError::ErrorType::InternalError);
    }

    co_return {};
}
coro::task<PermissionService::Result<void>>
    PermissionService::remove(const std::string user_name,
                              const std::string permission) {
    auto user = co_await m_repository.find_by_id_async(user_name);

    if (!user.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(user.error()), CrudError::ErrorType::EntityNotFound);
    }

    auto perms = user->permissions();
    perms.erase(permission);

    user->set_permissions(perms);

    auto task = m_repository.update_async(std::vector {*user});

    auto result = co_await task;

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(result.error()), CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<PermissionService::Result<std::vector<std::string>>>
    PermissionService::get(const std::string user_name) {
    auto user = co_await m_repository.find_by_id_async(user_name);

    if (!user.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(user.error()), CrudError::ErrorType::EntityNotFound);
    }

    std::vector<std::string> result;
    result.reserve(user->permissions().size());

    std::ranges::transform(
        user->permissions(), std::back_inserter(result),
        [](const Domain::Permission &result) { return std::string(result); });

    co_return result;
}

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
        for (const auto &permission : user->permissions()) {
            if (m_matcher.match(std::string(target_permissions[i]),
                                permission)) {
                matched_targets[i] = true;
                break;
            }
        }
    }

    co_return std::ranges::all_of(matched_targets, [](bool v) { return v; });
}

} // namespace bxt::Core::Application
