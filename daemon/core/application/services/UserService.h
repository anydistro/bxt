/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/UserDTO.h"
#include "core/application/errors/CrudError.h"
#include "core/domain/repositories/UserRepository.h"
#include "utilities/errors/Macro.h"

namespace bxt::Core::Application {
class UserService {
public:
    BXT_DECLARE_RESULT(CrudError)
    UserService(bxt::Core::Domain::UserRepository& repository,
                Domain::UnitOfWorkBaseFactory& uow_factory)
        : m_repository(repository)
        , m_uow_factory(uow_factory) {
    }

    virtual coro::task<Result<void>> add_user(UserDTO const user);
    virtual coro::task<Result<void>> remove_user(std::string const name);
    virtual coro::task<Result<void>> update_user(UserDTO const user);
    virtual coro::task<Result<std::vector<UserDTO>>> get_users() const;

private:
    bxt::Core::Domain::UserRepository& m_repository;
    Domain::UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::Core::Application
