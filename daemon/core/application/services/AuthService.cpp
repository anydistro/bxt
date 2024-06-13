/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "AuthService.h"

#include "core/application/errors/AuthError.h"
namespace bxt::Core::Application {

coro::task<AuthService::Result<void>> AuthService::auth(std::string name,
                                                        std::string password) {
    const auto entity = co_await m_user_repository.find_by_id_async(
        name, co_await m_uow_factory());

    if (!entity.has_value()) {
        co_return bxt::make_error<AuthError>(
            AuthError::ErrorType::UserNotFound);
    }

    if (entity->password() != password) {
        co_return bxt::make_error<AuthError>(
            AuthError::ErrorType::InvalidCredentials);
    }

    co_return {};
}

coro::task<AuthService::Result<void>>
    AuthService::verify(const std::string token) const {
}

} // namespace bxt::Core::Application
