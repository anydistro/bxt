/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "core/application/errors/AuthError.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "core/domain/repositories/UserRepository.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <coro/task.hpp>
#include <jwt-cpp/jwt.h>
#include <kangaru/autowire.hpp>
#include <string>

namespace bxt::Core::Application {

class AuthService {
public:
    BXT_DECLARE_RESULT(AuthError);

    AuthService(Domain::UserRepository& user_repository,
                Domain::UnitOfWorkBaseFactory& uow_factory)
        : m_user_repository(user_repository), m_uow_factory(uow_factory) {}

    coro::task<Result<void>> auth(std::string name, std::string password);

    coro::task<Result<void>> verify(const std::string token) const;

private:
    Domain::UserRepository& m_user_repository;
    Domain::UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::Core::Application
