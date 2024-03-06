/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "UserController.h"

#include "core/application/dtos/UserDTO.h"
#include "drogon/HttpTypes.h"
#include "utilities/drogon/Helpers.h"
#include "utilities/log/Logging.h"

#include <algorithm>
#include <iterator>
#include <json/value.h>

namespace bxt::Presentation {

drogon::Task<drogon::HttpResponsePtr>
    UserController::add_user(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("users.add", req)

    if (!req->getJsonObject()) {
        co_return drogon_helpers::make_error_response("Body is malformed");
    }
    auto json = *req->getJsonObject();

    Core::Application::UserDTO dto;

    if (!json["name"] || !json["name"].isString()) {
        co_return drogon_helpers::make_error_response(
            "No name for entity provided");
    }
    dto.name = json["name"].asString();

    if (json["password"] && json["password"].isString()) {
        dto.password = json["password"].asString();
    }

    if (json["permissions"] && json["permissions"].isArray()) {
        dto.permissions = std::set<std::string>();

        for (const auto& permission : json["permissions"]) {
            if (permission.isString()) {
                dto.permissions->emplace(permission.asString());
            } else {
                logw("User Controller: trying to add a permission with wrong "
                     "type");
            }
        }
    }

    const auto add_ok = co_await m_service.add_user(dto);

    if (!add_ok.has_value()) {
        co_return drogon_helpers::make_error_response(add_ok.error().what());
    }

    co_return drogon_helpers::make_ok_response();
}

drogon::Task<drogon::HttpResponsePtr>
    UserController::update_user(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("users.update", req)

    if (!req->getJsonObject()) {
        co_return drogon_helpers::make_error_response("Body is malformed");
    }

    auto json = *req->getJsonObject();

    Core::Application::UserDTO dto;

    if (!json["name"] || !json["name"].isString()) {
        co_return drogon_helpers::make_error_response(
            "No name for entity provided");
    }
    dto.name = json["name"].asString();

    if (json["password"] && json["password"].isString()) {
        dto.password = json["password"].asString();
    }

    if (json["permissions"] && json["permissions"].isArray()) {
        dto.permissions = std::set<std::string>();

        for (const auto& permission : json["permissions"]) {
            if (permission.isString()) {
                dto.permissions->emplace(permission.asString());
            } else {
                logw("User Controller: trying to add a permission with wrong "
                     "type");
            }
        }
    }

    const auto update_ok = co_await m_service.update_user(dto);

    if (!update_ok.has_value()) {
        co_return drogon_helpers::make_error_response(update_ok.error().what());
    }

    co_return drogon_helpers::make_ok_response();
}

drogon::Task<drogon::HttpResponsePtr>
    UserController::remove_user(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("users.remove", req)

    auto json = *req->getJsonObject();

    const auto id = json["id"].asString();
    Json::Value result;

    const auto remove_ok = co_await m_service.remove_user(id);

    if (!remove_ok.has_value()) {
        co_return drogon_helpers::make_error_response(remove_ok.error().what());
    }

    co_return drogon_helpers::make_ok_response();
}

drogon::Task<drogon::HttpResponsePtr>
    UserController::get_users(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("users.get", req)

    const auto users = co_await m_service.get_users();
    Json::Value result;

    if (!users.has_value()) {
        co_return drogon_helpers::make_error_response(users.error().what());
    }

    for (const auto& user : *users) {
        Json::Value user_json;

        user_json["name"] = user.name;

        user_json["permissions"] = Json::Value(Json::arrayValue);

        for (const auto& permission : *user.permissions) {
            user_json["permissions"].append(permission);
        }

        result.append(user_json);
    }

    co_return drogon::HttpResponse::newHttpJsonResponse(result);
}

} // namespace bxt::Presentation
