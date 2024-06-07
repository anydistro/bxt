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
        drogon_helpers::get_request_json<AuthRequest>(req, true);

    if (!auth_request) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Invalid request: {}", auth_request.error()->what()));
    }

    const auto& [name, password, response_type] = *auth_request;

    const auto check_ok = co_await m_service.auth(name, password);

    if (!check_ok.has_value()) {
        co_return drogon_helpers::make_error_response(check_ok.error().what(),
                                                      drogon::k401Unauthorized);
    }

    const auto token = jwt::create()
                           .set_payload_claim("username", name)
                           .set_payload_claim("storage", response_type)
                           .set_issuer("auth0")
                           .set_type("JWS")
                           .sign(jwt::algorithm::hs256 {m_options.secret});

    if (response_type == "bearer") {
        co_return drogon_helpers::make_json_response(
            AuthResponse {.access_token = token, .token_type = "bearer"}, true);
    } else if (response_type == "cookie") {
        drogon::Cookie jwt_cookie("token", token);
        jwt_cookie.setHttpOnly(true);

        auto response = drogon_helpers::make_ok_response();
        response->addCookie(jwt_cookie);
        co_return response;
    }
    co_return drogon_helpers::make_error_response(
        "Invalid response type requested", drogon::k400BadRequest);
}

drogon::Task<drogon::HttpResponsePtr>
    AuthController::verify(drogon::HttpRequestPtr req) {
    auto token = req->getCookie("token");
    auto provided_storage = "cookie";

    if (token.empty()) {
        auto bearer = req->getHeader("Authorization");
        provided_storage = "bearer";
        if (!bearer.empty() && bearer.substr(0, 7) == "Bearer ") {
            token = bearer.substr(7);
        }
    }

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

        if (!decoded.has_payload_claim("storage")) {
            co_return drogon_helpers::make_error_response(
                "No token storage provided", drogon::k401Unauthorized);
        }

        auto storage = decoded.get_payload_claim("storage").as_string();

        if (storage != provided_storage) {
            co_return drogon_helpers::make_error_response(
                fmt::format(
                    R"(Token storage is invalid, expected: "{}", got: "{}")",
                    provided_storage, storage),
                drogon::k401Unauthorized);
        }

        co_return drogon_helpers::make_ok_response();

    } catch (const std::exception& exception) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Token is invalid, the error is: \"{}\"",
                        exception.what()),
            drogon::k401Unauthorized);
    }
}

} // namespace bxt::Presentation
