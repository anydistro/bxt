/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#define BXT_ADD_METHOD_TO(...) ADD_METHOD_TO(__VA_ARGS__, drogon::Options)

#define BXT_JWT_ADD_METHOD_TO(...) \
    ADD_METHOD_TO(__VA_ARGS__, drogon::Options, "bxt::Presentation::JwtFilter")

#define BXT_JWT_CHECK_PERMISSIONS(permissions, requestptr)                     \
                                                                               \
    if (!co_await m_permission_service.check(                                  \
            permissions,                                                       \
            requestptr->getAttributes()->get<std::string>("jwt_username"))) {  \
        Json::Value result;                                                    \
        result["error"] = "Not sufficient permissions to perform this action"; \
        result["status"] = "error";                                            \
                                                                               \
        auto response = drogon::HttpResponse::newHttpJsonResponse(result);     \
        response->setStatusCode(drogon::k401Unauthorized);                     \
                                                                               \
        co_return response;                                                    \
    }
