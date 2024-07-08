/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "core/application/services/PermissionService.h"
#include "event_log/application/services/LogService.h"
#include "utilities/drogon/Macro.h"

#include <drogon/HttpController.h>

namespace bxt::Presentation {

class LogController : public drogon::HttpController<LogController, false> {
public:
    LogController(EventLog::Application::LogService &service,
                  Core::Application::PermissionService &permission_service)
        : m_service(service), m_permission_service(permission_service) {}

    METHOD_LIST_BEGIN
    BXT_JWT_ADD_METHOD_TO(LogController::get_package_logs,
                          "/api/logs?since={since}&until={until}&text={text}",
                          drogon::Get);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr>
        get_package_logs(drogon::HttpRequestPtr req,
                         const std::string &since,
                         const std::string &until,
                         const std::string &text);

private:
    EventLog::Application::LogService &m_service;
    Core::Application::PermissionService &m_permission_service;
};

} // namespace bxt::Presentation
