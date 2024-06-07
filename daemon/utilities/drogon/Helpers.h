/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "drogon/utils/FunctionTraits.h"
#include "utilities/reflect/PathParser.h"

#include "json/value.h"
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <rfl/SnakeCaseToCamelCase.hpp>
#include <rfl/json/read.hpp>
#include <rfl/json/write.hpp>

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

// Use reflect-cpp to make json response
template<typename T>
inline ::drogon::HttpResponsePtr make_json_response(const T& value,
                                                    bool raw = false) {
    auto result = ::drogon::HttpResponse::newHttpResponse();
    if (raw) {
        result->setBody(rfl::json::write(value));
    } else {
        result->setBody(rfl::json::write<rfl::SnakeCaseToCamelCase>(value));
    }
    result->setStatusCode(drogon::k200OK);

    return result;
}
template<typename T>
inline auto get_request_json(drogon::HttpRequestPtr req, bool raw = false) {
    if (raw) {
        return rfl::json::read<T>(std::string(req->body()));
    } else {
        return rfl::json::read<T, rfl::SnakeCaseToCamelCase>(
            std::string(req->body()));
    }
}

} // namespace bxt::drogon_helpers
