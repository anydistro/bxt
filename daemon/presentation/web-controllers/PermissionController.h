/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/services/PermissionService.h"
#include "utilities/drogon/Macro.h"

#include <drogon/HttpController.h>

namespace bxt::Presentation {

class PermissionController
    : public drogon::HttpController<PermissionController, false> {
public:
    PermissionController(Core::Application::PermissionService& service)
        : m_service(service) {};

    METHOD_LIST_BEGIN

    BXT_JWT_ADD_METHOD_TO(PermissionController::add_permission,
                          "/api/permissions/add",
                          drogon::Post);
    BXT_JWT_ADD_METHOD_TO(PermissionController::remove_permission,
                          "/api/permissions/remove",
                          drogon::Post);
    BXT_JWT_ADD_METHOD_TO(PermissionController::get_permissions,
                          "/api/permissions",
                          drogon::Get);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr>
        add_permission(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr>
        remove_permission(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr>
        get_permissions(drogon::HttpRequestPtr req);

private:
    Core::Application::PermissionService& m_service;
};

} // namespace bxt::Presentation
