/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "drogon/utils/FunctionTraits.h"

#include "json/value.h"
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>

namespace bxt::drogon_helpers {
inline ::drogon::HttpResponsePtr make_error_response(
    const std::string& message,
    ::drogon::HttpStatusCode code = ::drogon::k400BadRequest) {
    Json::Value result_json;

    result_json["status"] = "error";
    result_json["message"] = message;

    auto result = ::drogon::HttpResponse::newHttpJsonResponse(result_json);
    result->setStatusCode(code);

    return result;
}

inline ::drogon::HttpResponsePtr
    make_ok_response(const std::string& message = "") {
    Json::Value result_json;

    result_json["status"] = "ok";
    if (!message.empty()) { result_json["message"] = message; }

    auto result = ::drogon::HttpResponse::newHttpJsonResponse(result_json);
    result->setStatusCode(drogon::k200OK);

    return result;
}
} // namespace bxt::drogon_helpers
