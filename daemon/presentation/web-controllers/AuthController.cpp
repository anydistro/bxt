/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "AuthController.h"

#include "drogon/HttpTypes.h"
#include "utilities/drogon/Helpers.h"

#include <boost/json.hpp>
#include <exception>
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
                           .sign(jwt::algorithm::hs256 {m_options.secret});

    auto response = drogon::HttpResponse::newHttpResponse();
    drogon::Cookie jwt_cookie("token", token);

    jwt_cookie.setHttpOnly(true);

    response->addCookie(jwt_cookie);
    co_return response;
}

drogon::Task<drogon::HttpResponsePtr>
    AuthController::verify(drogon::HttpRequestPtr req) {
    const auto token = req->getCookie("token");
    if (token.empty()) {
        co_return drogon_helpers::make_error_response("Token is missing",
                                                      drogon::k401Unauthorized);
    }

    try {
        const auto decoded = jwt::decode(token);
        const auto verifier =
            jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256 {m_options.secret})
                .with_issuer("auth0");

        verifier.verify(decoded);

        co_return drogon_helpers::make_ok_response();

    } catch (const std::exception& exception) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Token is invalid, the error is: \"{}\"",
                        exception.what()),
            drogon::k401Unauthorized);
    }
}
} // namespace bxt::Presentation
