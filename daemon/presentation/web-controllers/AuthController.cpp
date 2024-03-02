/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "AuthController.h"

#include "drogon/HttpTypes.h"

#include <boost/json.hpp>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>

namespace bxt::Presentation {

drogon::Task<drogon::HttpResponsePtr>
    AuthController::auth(drogon::HttpRequestPtr req) {
    if (!req->getJsonObject() || !req->getJsonObject()->isObject()) {
        auto error_resp = drogon::HttpResponse::newHttpResponse();
        error_resp->setStatusCode(drogon::k400BadRequest);
        co_return error_resp;
    }

    auto json = *req->getJsonObject();

    const auto name_json = json["name"];
    const auto password_json = json["password"];

    if (name_json.isNull() || password_json.isNull()) {
        auto error_resp = drogon::HttpResponse::newHttpResponse();
        error_resp->setStatusCode(drogon::k400BadRequest);
        co_return error_resp;
    }

    const std::string name = name_json.asString();
    const std::string password = password_json.asString();

    if (!co_await m_service.auth(name, password)) {
        auto error_resp = drogon::HttpResponse::newHttpResponse();
        error_resp->setStatusCode(drogon::k401Unauthorized);
        co_return error_resp;
    }
    const auto token = jwt::create()
                           .set_payload_claim("username", name)
                           .set_issuer("auth0")
                           .set_type("JWS")
                           .sign(jwt::algorithm::hs256 {"secret"});

    auto response = drogon::HttpResponse::newHttpResponse();
    drogon::Cookie jwt_cookie("token", token);

    jwt_cookie.setHttpOnly(true);

    response->addCookie(jwt_cookie);
    co_return response;
}

drogon::Task<drogon::HttpResponsePtr>
    AuthController::verify(drogon::HttpRequestPtr req) {
    auto header = req->getHeader("Authorization");
    if (header.empty()) {
        auto error_resp = drogon::HttpResponse::newHttpResponse();
        error_resp->setStatusCode(drogon::k401Unauthorized);
        co_return error_resp;
    }

    auto token = header.substr(7);
    auto decoded = jwt::decode(token);
    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256 {"secret"})
                        .with_issuer("auth0");

    try {
        verifier.verify(decoded);

        co_return drogon::HttpResponse::newHttpJsonResponse({{"status", "ok"}});
    } catch (const jwt::token_verification_exception& verification_exception) {
        auto error_resp = drogon::HttpResponse::newHttpResponse();
        error_resp->setStatusCode(drogon::k401Unauthorized);
        co_return error_resp;
    }
}
} // namespace bxt::Presentation
