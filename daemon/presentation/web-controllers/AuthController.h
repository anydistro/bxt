/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/services/AuthService.h"
#include "presentation/JwtOptions.h"
#include "utilities/drogon/Macro.h"

#include <drogon/HttpController.h>

namespace bxt::Presentation {

class AuthController : public drogon::HttpController<AuthController, false> {
public:
    AuthController(Presentation::JwtOptions& options,
                   Core::Application::AuthService& service)
        : m_options(options), m_service(service) {}

    METHOD_LIST_BEGIN

    BXT_ADD_METHOD_TO(AuthController::auth, "/api/auth", drogon::Post);
    BXT_JWT_ADD_METHOD_TO(AuthController::verify, "/api/verify", drogon::Get);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> auth(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr> verify(drogon::HttpRequestPtr req);

private:
    Presentation::JwtOptions& m_options;
    Core::Application::AuthService& m_service;
};

} // namespace bxt::Presentation
