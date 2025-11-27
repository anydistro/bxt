/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include <coro/task.hpp>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>

#include "Helpers.hpp"
#include "Request.hpp"

#define BXT_ADD_METHOD_TO(...) ADD_METHOD_TO(__VA_ARGS__, drogon::Options)

#define BXT_JWT_ADD_METHOD_TO(...) \
    ADD_METHOD_TO(__VA_ARGS__, drogon::Options, "bxt::Presentation::JwtFilter")

coro::task<drogon::HttpResponsePtr>
    bxt_jwt_check_permissions(auto permission_service, auto permissions, auto request_ptr) {
    if (!co_await permission_service.check(
            permissions, request_ptr->getAttributes()->template get<std::string>("jwt_username"))) {
        co_return bxt::drogon_helpers::make_error_response(
            "Not sufficient permissions to perform this action", drogon::k403Forbidden);
    }
    co_return nullptr;
}

#define BXT_JWT_CHECK_PERMISSIONS(permissions, request_ptr)                                       \
                                                                                                  \
    if (const auto result =                                                                       \
            co_await bxt_jwt_check_permissions(m_permission_service, permissions, request_ptr)) { \
        co_return result;                                                                         \
    }

#define BXT_EXTRACT_REQUEST(parsed_req_type, http_req)                                 \
    ({                                                                                 \
        auto parsed_req_name = ParsedRequest<parsed_req_type>::from_request(http_req); \
        if (!parsed_req_name.request) {                                                \
            co_return make_error_response(parsed_req_name.request.error());            \
        }                                                                              \
        parsed_req_name.request.value();                                               \
    })
