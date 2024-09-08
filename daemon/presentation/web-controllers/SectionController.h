/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/services/PermissionService.h"
#include "core/application/services/SectionService.h"
#include "drogon/HttpController.h"
#include "drogon/utils/coroutine.h"
#include "drogon/utils/FunctionTraits.h"
#include "utilities/drogon/Macro.h"
namespace bxt::Presentation {

class SectionController : public drogon::HttpController<SectionController, false> {
public:
    SectionController(Core::Application::SectionService& service,
                      Core::Application::PermissionService& permission_service)
        : m_service(service)
        , m_permission_service(permission_service) {
    }

    METHOD_LIST_BEGIN

    BXT_JWT_ADD_METHOD_TO(SectionController::get_sections, "/api/sections", drogon::Get);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> get_sections(drogon::HttpRequestPtr req) const;

private:
    Core::Application::SectionService& m_service;
    Core::Application::PermissionService& m_permission_service;
};

} // namespace bxt::Presentation
