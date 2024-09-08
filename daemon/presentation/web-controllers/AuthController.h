/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/services/AuthService.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "drogon/HttpTypes.h"
#include "drogon/utils/FunctionTraits.h"
#include "presentation/JwtOptions.h"
#include "utilities/drogon/Macro.h"
#include "utilities/lmdb/Database.h"
#include "utilities/lmdb/Environment.h"

#include <drogon/HttpController.h>
#include <expected>
#include <memory>

namespace bxt::Presentation {

class AuthController : public drogon::HttpController<AuthController, false> {
public:
    AuthController(Presentation::JwtOptions& options,
                   Core::Application::AuthService& service,
                   std::shared_ptr<Utilities::LMDB::Environment> env,
                   Core::Domain::UnitOfWorkBaseFactory& uow_factory)
        : m_options(options)
        , m_service(service)
        , m_token_db(env, "bxt::Tokens")
        , m_uow_factory(uow_factory) {
    }

    METHOD_LIST_BEGIN

    BXT_ADD_METHOD_TO(AuthController::auth, "/api/auth", drogon::Post);
    BXT_ADD_METHOD_TO(AuthController::refresh, "/api/auth/refresh", drogon::Get);
    BXT_ADD_METHOD_TO(AuthController::revoke, "/api/auth/revoke", drogon::Post);

    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> auth(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr> refresh(drogon::HttpRequestPtr req);
    drogon::Task<drogon::HttpResponsePtr> revoke(drogon::HttpRequestPtr req);

private:
    drogon::HttpResponsePtr
        make_token_response(Token& access_token, Token& refresh_token, Token::Storage storage);
    Presentation::JwtOptions& m_options;
    Core::Application::AuthService& m_service;
    Utilities::LMDB::Database<bool> m_token_db;
    Core::Domain::UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::Presentation
