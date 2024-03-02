/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/services/DeploymentService.h"
#include "core/application/services/PackageService.h"
#include "core/application/services/PermissionService.h"
#include "core/application/services/SyncService.h"
#include "drogon/utils/FunctionTraits.h"
#include "drogon/utils/coroutine.h"
#include "utilities/drogon/Macro.h"

#include <drogon/drogon.h>
#include <kangaru/autowire.hpp>
#include <string>

namespace bxt::Presentation {

class PackageController
    : public drogon::HttpController<PackageController, false> {
public:
    PackageController(Core::Application::PackageService &package_service,
                      Core::Application::SyncService &sync_service,
                      Core::Application::PermissionService &permission_service)
        : m_package_service(package_service),
          m_sync_service(sync_service),
          m_permission_service(permission_service) {};

    METHOD_LIST_BEGIN

    BXT_JWT_ADD_METHOD_TO(PackageController::commit_transaction,
                          "/api/packages/commit",
                          drogon::Post);

    BXT_JWT_ADD_METHOD_TO(
        PackageController::get_packages,
        "/api/packages/get?branch={1}&repository={2}&architecture={3}",
        drogon::Get);

    BXT_JWT_ADD_METHOD_TO(PackageController::sync, "/api/sync", drogon::Post);

    BXT_JWT_ADD_METHOD_TO(PackageController::snap, "/api/snap", drogon::Post);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> sync(drogon::HttpRequestPtr req);

    drogon::Task<drogon::HttpResponsePtr>
        commit_transaction(drogon::HttpRequestPtr req);

    drogon::Task<drogon::HttpResponsePtr>
        get_packages(drogon::HttpRequestPtr req,
                     const std::string &branch,
                     const std::string &repository,
                     const std::string &architecture);

    drogon::Task<drogon::HttpResponsePtr> snap(drogon::HttpRequestPtr req);

private:
    Core::Application::PackageService &m_package_service;
    Core::Application::SyncService &m_sync_service;
    Core::Application::PermissionService &m_permission_service;
};

} // namespace bxt::Presentation
