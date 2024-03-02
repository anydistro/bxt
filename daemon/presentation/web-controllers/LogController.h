/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "core/application/services/PackageLogEntryService.h"
#include "utilities/drogon/Macro.h"

#include <drogon/HttpController.h>

namespace bxt::Presentation {

class LogController : public drogon::HttpController<LogController, false> {
public:
    LogController(Core::Application::PackageLogEntryService &service)
        : m_service(service) {}

    METHOD_LIST_BEGIN

    BXT_JWT_ADD_METHOD_TO(LogController::get_package_logs,
                          "/api/logs/packages",
                          drogon::Get);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr>
        get_package_logs(drogon::HttpRequestPtr);

private:
    Core::Application::PackageLogEntryService &m_service;
};

} // namespace bxt::Presentation
