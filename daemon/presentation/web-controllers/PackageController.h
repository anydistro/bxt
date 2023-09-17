/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/services/DeploymentService.h"
#include "core/application/services/PackageService.h"
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
    PackageController(Core::Application::PackageService &package_service)
        : m_package_service(package_service) {};

    METHOD_LIST_BEGIN

    BXT_JWT_ADD_METHOD_TO(PackageController::commit_transaction,
                          "/api/packages/commit",
                          drogon::Post);

    BXT_JWT_ADD_METHOD_TO(
        PackageController::get_packages,
        "/api/packages/get?branch={1}&repository={2}&architecture={3}",
        drogon::Get);

    BXT_JWT_ADD_METHOD_TO(PackageController::sync, "/api/sync", drogon::Post);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> sync(drogon::HttpRequestPtr req);

    drogon::Task<drogon::HttpResponsePtr>
        commit_transaction(drogon::HttpRequestPtr req);

    drogon::Task<drogon::HttpResponsePtr>
        get_packages(drogon::HttpRequestPtr req,
                     const std::string &branch,
                     const std::string &repository,
                     const std::string &architecture);

private:
    Core::Application::PackageService &m_package_service;
};

} // namespace bxt::Presentation
