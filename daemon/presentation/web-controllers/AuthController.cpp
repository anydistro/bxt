/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "AuthController.h"

#include "drogon/HttpTypes.h"
#include "presentation/messages/AuthMessages.h"
#include "utilities/drogon/Helpers.h"

#include <boost/json.hpp>
#include <exception>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>

namespace bxt::Presentation {

drogon::Task<drogon::HttpResponsePtr>
    AuthController::auth(drogon::HttpRequestPtr req) {
    const auto auth_request =
        drogon_helpers::get_request_json<AuthRequest>(req);

    if (!auth_request) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Invalid request: {}", auth_request.error()->what()));
    }

    const auto& [name, password] = *auth_request;

    const auto check_ok = co_await m_service.auth(name, password);

    if (!check_ok.has_value()) {
        co_return drogon_helpers::make_error_response(check_ok.error().what(),
                                                      drogon::k401Unauthorized);
    }

    const auto token = jwt::create()
                           .set_payload_claim("username", name)
                           .set_issuer("auth0")
                           .set_type("JWS")
                           .sign(jwt::algorithm::hs256 {m_options.secret});
    drogon::Cookie jwt_cookie("token", token);
    jwt_cookie.setHttpOnly(true);

    auto response = drogon::HttpResponse::newHttpResponse();
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
