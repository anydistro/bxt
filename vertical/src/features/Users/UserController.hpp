/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>
#include <sqlgen/sqlite.hpp>
#include <sqlgen/sqlite/Connection.hpp>

#include "infra/drogon/Macro.hpp"
#include "infra/sqlgen/SqlgenUnitOfWorkFactory.hpp"

namespace bxt::Users {
class UserController : public drogon::HttpController<UserController, false> {
public:
    UserController(adapters::sqlgen::SqlgenUnitOfWorkFactory& uow_factory)
        : m_uow_factory(uow_factory) {
    }

    METHOD_LIST_BEGIN

    ADD_METHOD_TO(UserController::create_user, "/api/users", drogon::Post);

    ADD_METHOD_TO(UserController::update_user, "/api/users/{1}", drogon::Put);

    ADD_METHOD_TO(UserController::delete_user, "/api/users/{1}", drogon::Delete);

    ADD_METHOD_TO(UserController::get_users, "/api/users", drogon::Get);

    ADD_METHOD_TO(UserController::get_user, "/api/users/{1}", drogon::Get);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> create_user(drogon::HttpRequestPtr req);

    drogon::Task<drogon::HttpResponsePtr> update_user(drogon::HttpRequestPtr req,
                                                      std::string user_id);

    drogon::Task<drogon::HttpResponsePtr> delete_user(drogon::HttpRequestPtr req,
                                                      std::string user_id);

    drogon::Task<drogon::HttpResponsePtr> get_users(drogon::HttpRequestPtr req);

    drogon::Task<drogon::HttpResponsePtr> get_user(drogon::HttpRequestPtr req, std::string user_id);

private:
    adapters::sqlgen::SqlgenUnitOfWorkFactory& m_uow_factory;
};

} // namespace bxt::Users
