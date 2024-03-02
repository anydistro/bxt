/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "UserController.h"

#include "core/application/dtos/UserDTO.h"
#include "drogon/HttpTypes.h"

namespace bxt::Presentation {

drogon::Task<drogon::HttpResponsePtr>
    UserController::add_user(drogon::HttpRequestPtr req) {
    auto json = *req->getJsonObject();

    const auto name = json["name"].asString();
    const auto password = json["password"].asString();

    bxt::Core::Application::UserDTO dto {name, password, {}};
    Json::Value result;

    const auto add_ok = co_await m_service.add_user(dto);

    if (!add_ok.has_value()) {
        result["status"] = "error";
        result["error"] = add_ok.error().what();
        auto response = drogon::HttpResponse::newHttpJsonResponse(result);
        response->setStatusCode(drogon::k400BadRequest);
        co_return response;
    }

    result["status"] = "ok";
    co_return drogon::HttpResponse::newHttpJsonResponse(result);
}

drogon::Task<drogon::HttpResponsePtr>
    UserController::remove_user(drogon::HttpRequestPtr req) {
    auto json = *req->getJsonObject();

    const auto id = json["id"].asString();
    Json::Value result;

    const auto remove_ok = co_await m_service.remove_user(id);

    if (!remove_ok.has_value()) {
        result["status"] = "error";
        result["error"] = remove_ok.error().what();
        auto response = drogon::HttpResponse::newHttpJsonResponse(result);
        response->setStatusCode(drogon::k400BadRequest);
        co_return response;
    }

    result["status"] = "ok";
    co_return drogon::HttpResponse::newHttpJsonResponse(result);
}

drogon::Task<drogon::HttpResponsePtr>
    UserController::get_users(drogon::HttpRequestPtr req) {
    const auto users = co_await m_service.get_users();
    Json::Value result;

    if (!users.has_value()) {
        result["status"] = "error";
        result["error"] = users.error().what();
        auto response = drogon::HttpResponse::newHttpJsonResponse(result);
        response->setStatusCode(drogon::k400BadRequest);
        co_return response;
    }

    for (const auto& user : *users) {
        Json::Value user_json;

        user_json["name"] = user.name;

        result.append(user_json);
    }

    co_return drogon::HttpResponse::newHttpJsonResponse(result);
}

} // namespace bxt::Presentation
