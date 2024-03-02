/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "UserService.h"

#include "core/application/errors/CrudError.h"
namespace bxt::Core::Application {
coro::task<UserService::Result<void>>
    UserService::add_user(const UserDTO user) {
    const auto user_entity = UserDTOMapper::to_entity(user);

    auto result = co_await m_repository.add_async(user_entity);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(result.error()), CrudError::ErrorType::InternalError);
    }
    auto commit_ok = co_await m_repository.commit_async();

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

    auto result = co_await m_repository.remove_async(name);

    if (!result.has_value()) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(result.error()), CrudError::ErrorType::InternalError);
    }

    auto commit_ok = co_await m_repository.commit_async();

    if (!commit_ok) {
        co_return bxt::make_error_with_source<CrudError>(
            std::move(commit_ok.error()), CrudError::ErrorType::InternalError);
    }

    co_return {};
}

coro::task<UserService::Result<std::vector<UserDTO>>>
    UserService::get_users() const {
    std::vector<bxt::Core::Application::UserDTO> result;

    auto values = co_await m_repository.all_async();
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
