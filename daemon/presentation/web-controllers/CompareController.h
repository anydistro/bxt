/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/services/CompareService.h"
#include "core/application/services/DeploymentService.h"
#include "core/application/services/PackageService.h"
#include "drogon/utils/FunctionTraits.h"
#include "drogon/utils/coroutine.h"
#include "utilities/drogon/Macro.h"

#include <drogon/drogon.h>
#include <kangaru/autowire.hpp>
#include <string>

namespace bxt::Presentation {

class CompareController
    : public drogon::HttpController<CompareController, false> {
public:
    CompareController(Core::Application::CompareService &compare_service)
        : m_compare_service(compare_service) {};

    METHOD_LIST_BEGIN

    BXT_JWT_ADD_METHOD_TO(CompareController::compare,
                          "/api/compare",
                          drogon::Post);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> compare(drogon::HttpRequestPtr req);

private:
    Core::Application::CompareService &m_compare_service;
};

} // namespace bxt::Presentation
