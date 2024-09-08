/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "core/application/services/PermissionService.h"
#include "core/application/services/UserService.h"
#include "drogon/HttpTypes.h"
#include "utilities/drogon/Macro.h"

#include <drogon/HttpController.h>
#include <kangaru/service.hpp>

namespace bxt::Presentation {

class UserController : public drogon::HttpController<UserController, false> {
public:
    explicit UserController(Core::Application::UserService& service,
                            Core::Application::PermissionService& permission_service)
        : m_service(service)
        , m_permission_service(permission_service) {};

    METHOD_LIST_BEGIN

    BXT_JWT_ADD_METHOD_TO(UserController::add_user, "/api/users/add", drogon::Post);

    BXT_JWT_ADD_METHOD_TO(UserController::update_user, "/api/users/update", drogon::Patch);

    BXT_JWT_ADD_METHOD_TO(UserController::remove_user, "/api/users/remove/{1}", drogon::Delete);

    BXT_JWT_ADD_METHOD_TO(UserController::get_users, "/api/users", drogon::Get);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> add_user(drogon::HttpRequestPtr req);

    drogon::Task<drogon::HttpResponsePtr> update_user(drogon::HttpRequestPtr req);

    drogon::Task<drogon::HttpResponsePtr> remove_user(drogon::HttpRequestPtr req,
                                                      std::string user_name);

    drogon::Task<drogon::HttpResponsePtr> get_users(drogon::HttpRequestPtr req);

private:
    Core::Application::UserService& m_service;
    Core::Application::PermissionService& m_permission_service;
};

} // namespace bxt::Presentation
