/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/services/DeploymentService.h"
#include "core/application/services/PackageService.h"
#include "drogon/utils/FunctionTraits.h"
#include "drogon/utils/coroutine.h"
#include "presentation/cli-controllers/DeploymentOptions.h"
#include "utilities/drogon/Macro.h"

#include <drogon/drogon.h>
#include <kangaru/autowire.hpp>
#include <string>

namespace bxt::Presentation {

class DeploymentController
    : public drogon::HttpController<DeploymentController, false> {
public:
    DeploymentController(DeploymentOptions& options,
                         Core::Application::DeploymentService& service)
        : m_options(options), m_service(service) {};

    METHOD_LIST_BEGIN

    BXT_ADD_METHOD_TO(DeploymentController::deploy_start,
                      "/api/deploy/start",
                      drogon::Post);
    BXT_ADD_METHOD_TO(DeploymentController::deploy_push,
                      "/api/deploy/push",
                      drogon::Post);
    BXT_ADD_METHOD_TO(DeploymentController::deploy_end,
                      "/api/deploy/end",
                      drogon::Post);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr>
        deploy_start(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr>
        deploy_push(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr>
        deploy_end(drogon::HttpRequestPtr req);

private:
    DeploymentOptions& m_options;
    Core::Application::DeploymentService& m_service;
};

} // namespace bxt::Presentation
