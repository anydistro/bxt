/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "UserService.h"

#include "core/application/errors/CrudError.h"
#include "utilities/Error.h"

#include <algorithm>
namespace bxt::Core::Application {
coro::task<UserService::Result<void>>
    UserService::add_user(const UserDTO user) {
    const auto user_entity = UserDTOMapper::to_entity(user);

    auto uow = co_await m_uow_factory(true);

    auto result = co_await m_repository.save_async(user_entity, uow);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(result.error()), CrudError::ErrorType::InternalError);
    }
    auto commit_ok = co_await uow->commit_async();

    if (!commit_ok) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(commit_ok.error()), CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<UserService::Result<void>>
    UserService::remove_user(const std::string name) {
    if (name == "default") {
        co_return bxt::make_error<CrudError>(
            CrudError::ErrorType::InvalidArgument);
    }

    auto uow = co_await m_uow_factory(true);

    auto result = co_await m_repository.delete_async(name, uow);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(result.error()), CrudError::ErrorType::InternalError);
    }

    auto commit_ok = co_await uow->commit_async();

    if (!commit_ok) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(commit_ok.error()), CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<UserService::Result<void>>
    UserService::update_user(const UserDTO user) {
    auto uow = co_await m_uow_factory(true);

    auto existing_user_entity =
        co_await m_repository.find_by_id_async(user.name, uow);

    if (!existing_user_entity) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(existing_user_entity.error()),
            CrudError::ErrorType::EntityNotFound);
    }

    if (user.password) { existing_user_entity->set_password(*user.password); }

    if (user.permissions) {
        std::set<Domain::Permission> permission_entities;

        std::ranges::transform(
            *user.permissions,
            std::inserter(permission_entities, permission_entities.end()),
            [](const auto& p) { return Domain::Permission(p); });

        existing_user_entity->set_permissions(permission_entities);
    }

    auto result = co_await m_repository.save_async(*existing_user_entity, uow);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(result.error()), CrudError::ErrorType::InternalError);
    }
    auto commit_ok = co_await uow->commit_async();

    if (!commit_ok) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(commit_ok.error()), CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<UserService::Result<std::vector<UserDTO>>>
    UserService::get_users() const {
    std::vector<bxt::Core::Application::UserDTO> result;

    auto values = co_await m_repository.all_async(co_await m_uow_factory());
    if (!values.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(values.error()), CrudError::ErrorType::InternalError);
    }

    result.reserve(values->size());
    std::ranges::transform(*values, std::back_inserter(result),
                           UserDTOMapper::to_dto);

    co_return result;
}
} // namespace bxt::Core::Application
