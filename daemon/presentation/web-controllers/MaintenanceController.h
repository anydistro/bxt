/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/services/MaintenanceService.h"
#include "core/application/services/PermissionService.h"
#include "utilities/drogon/Macro.h"

#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <kangaru/service.hpp>

namespace bxt::Presentation {

class MaintenanceController : public drogon::HttpController<MaintenanceController, false> {
public:
    explicit MaintenanceController(Core::Application::MaintenanceService& service,
                                   Core::Application::PermissionService& permission_service)
        : m_service(service)
        , m_permission_service(permission_service) {
    }

    METHOD_LIST_BEGIN

    BXT_JWT_ADD_METHOD_TO(MaintenanceController::export_database,
                          "/api/maintenance/export",
                          drogon::Get);
    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> export_database(drogon::HttpRequestPtr req);

private:
    Core::Application::MaintenanceService& m_service;
    Core::Application::PermissionService& m_permission_service;
};

} // namespace bxt::Presentation
