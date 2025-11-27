/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <expected>
#include <tuple>

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <json/value.h>
#include <rfl/json/read.hpp>
#include <rfl/json/write.hpp>
#include <rfl/SnakeCaseToCamelCase.hpp>

namespace bxt::drogon_helpers {
inline ::drogon::HttpResponsePtr
    make_error_response(std::string const& message,
                        ::drogon::HttpStatusCode code = ::drogon::k400BadRequest) {
    Json::Value result_json;

    result_json["status"] = "error";
    result_json["message"] = message;

    auto result = ::drogon::HttpResponse::newHttpJsonResponse(result_json);
    result->setStatusCode(code);

    return result;
}

inline ::drogon::HttpResponsePtr make_ok_response(std::string const& message = "") {
    Json::Value result_json;

    result_json["status"] = "ok";
    if (!message.empty()) {
        result_json["message"] = message;
    }

    auto result = ::drogon::HttpResponse::newHttpJsonResponse(result_json);
    result->setStatusCode(drogon::k200OK);

    return result;
}

// Use reflect-cpp to make json response

template<typename T>
inline ::drogon::HttpResponsePtr make_json_response(T const& value, bool raw = false)
    requires(std::same_as<T, std::expected<typename T::value_type, typename T::error_type>>)
{
    auto result = ::drogon::HttpResponse::newHttpResponse();
    if (value) {
        if (raw) {
            result->setBody(rfl::json::write(value.value()));
        } else {
            result->setBody(rfl::json::write<rfl::SnakeCaseToCamelCase>(value.value()));
        }
    } else {
        result->setBody(rfl::json::write(value.error().what()));
        result->setStatusCode(drogon::k400BadRequest);
    }
    return result;
}

template<typename T>
inline ::drogon::HttpResponsePtr make_json_response(T const& value, bool raw = false) {
    auto result = ::drogon::HttpResponse::newHttpResponse();
    if (raw) {
        result->setBody(rfl::json::write(value));
    } else {
        result->setBody(rfl::json::write<rfl::SnakeCaseToCamelCase>(value));
    }
    result->setStatusCode(drogon::k200OK);

    return result;
}

template<typename T> inline auto get_request_json(drogon::HttpRequestPtr req, bool raw = false) {
    if (raw) {
        return rfl::json::read<T>(std::string(req->body()));
    } else {
        return rfl::json::read<T, rfl::SnakeCaseToCamelCase>(std::string(req->body()));
    }
}
struct PaginationLimits {
    std::size_t MinPage;
    std::size_t DefaultLimit;
    std::size_t MinLimit;
    std::size_t MaxLimit;
};

template<PaginationLimits Params> inline auto get_pagination(drogon::HttpRequestPtr req) {
    if (!req) {
        return std::make_tuple(Params.MinPage, Params.DefaultLimit, 0ul);
    }

    std::size_t const page = [req] {
        std::size_t const page =
            req->getOptionalParameter<std::size_t>("page").value_or(Params.MinPage);

        return std::max(page, Params.MinPage);
    }();

    std::size_t const limit = [req] {
        std::size_t const limit =
            req->getOptionalParameter<std::size_t>("limit").value_or(Params.DefaultLimit);

        return std::clamp(limit, Params.MinLimit, Params.MaxLimit);
    }();

    std::size_t const offset = (page - 1) * limit;

    return std::make_tuple(page, limit, offset);
}
// inline std::expected<Presentation::Token, std::string> get_access_token(drogon::HttpRequestPtr
//                                                                         std::string const&
//                                                                         issuer, std::string
//                                                                         const& secret) {
//     // First, try to get the access token from the cookie
//     auto token_str = req->getCookie(Presentation::Names::AccessToken);
//     if (!token_str.empty()) {
//         auto token = Presentation::Token::verify_jwt(token_str, issuer, secret);
//         if (!token) {
//             return std::unexpected(token.error());
//         }
//         if (token->storage() == Presentation::Token::Storage::Cookie) {
//             return token;
//         } else {
//             return std::unexpected("Invalid storage for access token");
//         }
//     }

//     // If the cookie is empty, try to get the token from the Authorization
//     // header
//     auto bearer = req->getHeader("Authorization");
//     if (!bearer.empty() && bearer.substr(0, 7) == "Bearer ") {
//         token_str = bearer.substr(7);
//         auto token = Presentation::Token::verify_jwt(token_str, issuer, secret);
//         if (!token) {
//             return std::unexpected(token.error());
//         }
//         if (token->storage() == Presentation::Token::Storage::Bearer) {
//             return token;
//         } else {
//             return std::unexpected("Invalid storage for access token");
//         }
//     }

//     return std::unexpected("No token found");
// }
// inline std::expected<Presentation::Token, std::string> get_refresh_token(
//     drogon::HttpRequestPtr req, std::string const& issuer, std::string const& secret) {
//     // First, try to get the refresh token from the cookie
//     auto token_str = req->getCookie(Presentation::Names::RefreshToken);
//     if (!token_str.empty()) {
//         auto token = Presentation::Token::verify_jwt(token_str, issuer, secret);
//         if (!token) {
//             return std::unexpected(token.error());
//         }
//         if (token->storage() == Presentation::Token::Storage::Cookie) {
//             return token;
//         } else {
//             return std::unexpected("Invalid storage for refresh token");
//         }
//     }

//     // If the cookie is empty, try to get the token from the request body
//     auto json = req->getJsonObject();
//     if (json) {
//         auto token_str = json->get("token", Json::Value()).asString();
//         if (!token_str.empty()) {
//             auto token = Presentation::Token::verify_jwt(token_str, issuer, secret);
//             if (!token) {
//                 return std::unexpected(token.error());
//             }
//             if (token->storage() == Presentation::Token::Storage::Bearer) {
//                 return token;
//             } else {
//                 return std::unexpected("Invalid storage for refresh token");
//             }
//         }
//     }

//     return std::unexpected("No token found");
// }

} // namespace bxt::drogon_helpers
