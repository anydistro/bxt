/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "UserController.h"

#include "core/application/dtos/UserDTO.h"
#include "presentation/messages/UserMessages.h"
#include "utilities/drogon/Helpers.h"

#include <json/value.h>
#include <rfl/as.hpp>
#include <rfl/json/read.hpp>

namespace bxt::Presentation {

drogon::Task<drogon::HttpResponsePtr>
    UserController::add_user(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("users.add", req)

    const auto user_request =
        drogon_helpers::get_request_json<AddUserRequest>(req);

    if (!user_request) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Invalid request: {}", user_request.error()->what()));
    }

    auto dto = rfl::as<Core::Application::UserDTO>(*user_request);

    const auto add_ok = co_await m_service.add_user(dto);

    if (!add_ok.has_value()) {
        co_return drogon_helpers::make_error_response(add_ok.error().what());
    }

    co_return drogon_helpers::make_ok_response();
}

drogon::Task<drogon::HttpResponsePtr>
    UserController::update_user(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("users.update", req)

    const auto user_request =
        drogon_helpers::get_request_json<UpdateUserRequest>(req);

    if (!user_request) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Invalid request: {}", user_request.error()->what()));
    }

    auto dto = rfl::as<Core::Application::UserDTO>(*user_request);

    const auto update_ok = co_await m_service.update_user(dto);

    if (!update_ok.has_value()) {
        co_return drogon_helpers::make_error_response(update_ok.error().what());
    }

    co_return drogon_helpers::make_ok_response();
}

drogon::Task<drogon::HttpResponsePtr>
    UserController::remove_user(drogon::HttpRequestPtr req,
                                std::string user_name) {
    BXT_JWT_CHECK_PERMISSIONS("users.remove", req)

    if (user_name.empty()) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Invalid user name"));
    }

    const auto remove_ok = co_await m_service.remove_user(user_name);

    if (!remove_ok.has_value()) {
        co_return drogon_helpers::make_error_response(remove_ok.error().what());
    }

    co_return drogon_helpers::make_ok_response();
}

drogon::Task<drogon::HttpResponsePtr>
    UserController::get_users(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("users.get", req)

    const auto users = co_await m_service.get_users();

    if (!users.has_value()) {
        co_return drogon_helpers::make_error_response(users.error().what());
    }

    co_return drogon_helpers::make_json_response(
        *users
        | std::views::transform([](const Core::Application::UserDTO& dto) {
              auto&& [name, password, permissions] = dto;
              return UserResponse {
                  name, permissions.value_or(std::set<std::string> {})};
          })
        | std::ranges::to<std::vector>());
}

} // namespace bxt::Presentation
