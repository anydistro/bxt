/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "AuthController.h"

#include "drogon/HttpTypes.h"
#include "persistence/lmdb/LmdbUnitOfWork.h"
#include "presentation/Names.h"
#include "presentation/Token.h"
#include "presentation/messages/AuthMessages.h"
#include "utilities/drogon/Helpers.h"

#include <boost/json.hpp>
#include <chrono>
#include <exception>
#include <fmt/core.h>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <memory>

namespace bxt::Presentation {

drogon::Task<drogon::HttpResponsePtr>
    AuthController::auth(drogon::HttpRequestPtr req) {
    const auto auth_request =
        drogon_helpers::get_request_json<AuthRequest>(req, true);

    if (!auth_request) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Invalid request: {}", auth_request.error()->what()));
    }

    const auto& [name, password, response_type_name] = *auth_request;

    Token::Storage response_type = Token::Storage::Cookie;
    if (response_type_name == Names::BearerStorage) {
        response_type = Token::Storage::Bearer;
    } else if (response_type_name == Names::CookieStorage) {
        response_type = Token::Storage::Cookie;
    } else {
        co_return drogon_helpers::make_error_response(
            fmt::format("Invalid response type: {}", response_type));
    }

    const auto check_ok = co_await m_service.auth(name, password);

    if (!check_ok.has_value()) {
        co_return drogon_helpers::make_error_response(check_ok.error().what(),
                                                      drogon::k401Unauthorized);
    }

    Token access_token {name, Token::Kind::Access, response_type};

    Token refresh_token {name, Token::Kind::Refresh, response_type};

    auto lmdb_uow = std::dynamic_pointer_cast<Persistence::LmdbUnitOfWork>(
        co_await m_uow_factory(true));

    if (!lmdb_uow) {
        co_return drogon_helpers::make_error_response(
            "Internal server error", drogon::k500InternalServerError);
    }

    auto put_ok = co_await m_token_db.put(
        lmdb_uow->txn().value,
        refresh_token.generate_jwt(m_options.issuer, m_options.secret), true);

    if (!put_ok.has_value()) {
        co_return drogon_helpers::make_error_response(
            put_ok.error().what(), drogon::k500InternalServerError);
    }

    if (!*put_ok || !co_await lmdb_uow->commit_async()) {
        co_return drogon_helpers::make_error_response(
            "Internal server error", drogon::k500InternalServerError);
    }

    co_return make_token_response(access_token, refresh_token, response_type);
}

drogon::Task<drogon::HttpResponsePtr>
    AuthController::refresh(drogon::HttpRequestPtr req) {
    auto refresh_token = drogon_helpers::get_refresh_token(
        req, m_options.issuer, m_options.secret);
    if (!refresh_token.has_value()) {
        co_return drogon_helpers::make_error_response(refresh_token.error(),
                                                      drogon::k401Unauthorized);
    }

    // jwt is cached in this case already
    auto refresh_token_jwt =
        refresh_token->generate_jwt(m_options.issuer, m_options.secret);

    if (refresh_token_jwt.empty()) {
        co_return drogon_helpers::make_error_response("Token is missing",
                                                      drogon::k401Unauthorized);
    }

    auto lmdb_uow = std::dynamic_pointer_cast<Persistence::LmdbUnitOfWork>(
        co_await m_uow_factory());

    if (!lmdb_uow) {
        co_return drogon_helpers::make_error_response(
            "Internal server error", drogon::k500InternalServerError);
    }

    auto get_ok =
        co_await m_token_db.get(lmdb_uow->txn().value, refresh_token_jwt);

    if (!get_ok.has_value()) {
        co_return drogon_helpers::make_error_response(
            get_ok.error().what(), drogon::k500InternalServerError);
    }

    if (!*get_ok) {
        co_return drogon_helpers::make_error_response("Token is invalid",
                                                      drogon::k401Unauthorized);
    }

    Token new_access_token {refresh_token->name(), Token::Kind::Access,
                            refresh_token->storage()};

    co_return make_token_response(new_access_token, *refresh_token,
                                  refresh_token->storage());
}
drogon::Task<drogon::HttpResponsePtr>
    AuthController::revoke(drogon::HttpRequestPtr req) {
    auto refresh_token = drogon_helpers::get_refresh_token(
        req, m_options.issuer, m_options.secret);
    if (!refresh_token.has_value()) {
        co_return drogon_helpers::make_error_response(refresh_token.error(),
                                                      drogon::k401Unauthorized);
    }

    auto refresh_token_jwt =
        refresh_token->generate_jwt(m_options.issuer, m_options.secret);

    if (refresh_token_jwt.empty()) {
        co_return drogon_helpers::make_error_response("Token is missing",
                                                      drogon::k401Unauthorized);
    }

    auto lmdb_uow = std::dynamic_pointer_cast<Persistence::LmdbUnitOfWork>(
        co_await m_uow_factory(true));

    if (!lmdb_uow) {
        co_return drogon_helpers::make_error_response(
            "Internal server error", drogon::k500InternalServerError);
    }

    if (!co_await m_token_db.del(lmdb_uow->txn().value, refresh_token_jwt)) {
        co_return drogon_helpers::make_error_response(
            "Failed to revoke token", drogon::k500InternalServerError);
    }

    auto response = drogon_helpers::make_ok_response("Token revoked");

    response->removeCookie(Names::RefreshToken);
    response->removeCookie(Names::AccessToken);

    co_return response;
}

auto time_point_to_trantor_date(
    const std::chrono::system_clock::time_point& time) {
    auto microseconds_since_epoch =
        std::chrono::duration_cast<std::chrono::microseconds>(
            time.time_since_epoch());
    return trantor::Date {microseconds_since_epoch.count()};
}

drogon::HttpResponsePtr AuthController::make_token_response(
    Token& access_token, Token& refresh_token, Token::Storage storage) {
    auto access_jwt =
        access_token.generate_jwt(m_options.issuer, m_options.secret);
    auto refresh_jwt =
        refresh_token.generate_jwt(m_options.issuer, m_options.secret);

    if (storage == Token::Bearer) {
        return drogon_helpers::make_json_response(
            Presentation::AuthResponse {.access_token = access_jwt,
                                        .refresh_token = refresh_jwt,
                                        .token_type =
                                            Presentation::Names::BearerStorage},
            true);
    } else if (storage == Token::Cookie) {
        drogon::Cookie access_cookie(Presentation::Names::AccessToken,
                                     access_jwt);

        access_cookie.setHttpOnly(true);
        access_cookie.setPath("/api");
        access_cookie.setExpiresDate(
            time_point_to_trantor_date(access_token.expires_at()));

        drogon::Cookie refresh_cookie(Presentation::Names::RefreshToken,
                                      refresh_jwt);
        refresh_cookie.setHttpOnly(true);
        refresh_cookie.setPath("/api/auth");
        refresh_cookie.setExpiresDate(
            time_point_to_trantor_date(refresh_token.expires_at()));

        auto response = drogon_helpers::make_ok_response();
        response->addCookie(access_cookie);
        response->addCookie(refresh_cookie);
        return response;
    }
    return drogon_helpers::make_error_response(
        "Invalid response type requested", drogon::k400BadRequest);
}

} // namespace bxt::Presentation
