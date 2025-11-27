/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "UserController.hpp"

#include <ranges>
#include <string>

#include <coro/when_all.hpp>
#include <drogon/HttpTypes.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <rfl/as.hpp>

#include "domain/User.hpp"
#include "infra/drogon/Helpers.hpp"
#include "infra/drogon/Macro.hpp"
#include "infra/drogon/ToJsonResponse.hpp"
#include "infra/drogon/types.hpp"
#include "core/result_types.hpp"
#include "core/types/Path.hpp"
#include "core/types/TimePoint.hpp"
#include "core/types/to_string.hpp"
#include "core/domain/value_objects/Name.hpp"
#include "infra/logging/Logging.hpp"
#include "utils/ranges/ConvertView.hpp"
#include "UserConvert.hpp"
#include "UserMessages.hpp"
#include "domain/Permission.hpp"

namespace bxt::Users {

using namespace drogon_helpers;
ResponseTask UserController::create_user(drogon::HttpRequestPtr raw_request) {
    auto request = BXT_EXTRACT_REQUEST(AddUserRequest, raw_request);

    auto user = domain::User::create(domain::value_objects::Name::create(request.name).value(),
                                     request.password);

    for (auto permission : request.permissions.value_or(std::set<std::string> {})) {
        user->grant_permission(domain::Permission::create(permission).value());
    }

    if (auto [uow, err, repository] = co_await m_uow_factory.with_rw_repositories<domain::User>();
        !err) {
        auto result = repository.create(*user);
        if (!result) {
            bxt::loge("Error creating user: {}", result.error().what());
            std::string error_msg;
            if (request.permissions && !request.permissions->empty()) {
                error_msg = fmt::format("Failed to create user '{}' with permissions [{}]",
                                        request.name, fmt::join(*request.permissions, ", "));
            } else {
                error_msg = fmt::format("Failed to create user '{}'", request.name);
            }

            co_return make_error_response(error_msg, drogon::k409Conflict);
        }

        co_return make_json_response(utils::convert_to<UserResponse, domain::User> {}(*result));
    } else {
        co_return make_error_response(
            fmt::format("Database transaction initialization failed during user creation for '{}'",
                        request.name),
            drogon::k503ServiceUnavailable);
    }
}

ResponseTask UserController::update_user(drogon::HttpRequestPtr req, std::string user_id) {
    auto parsed = BXT_EXTRACT_REQUEST(UpdateUserRequest, req);

    if (auto [uow, err, repository] = co_await m_uow_factory.with_rw_repositories<domain::User>();
        !err) {
        auto existing = repository.get_by_id(user_id);
        if (!existing) {
            co_return make_error_response(fmt::format("User with ID '{}' not found", user_id),
                                          drogon::k404NotFound);
        }

        if (parsed.password) {
            existing->change_password(*parsed.password);
        }
        if (parsed.permissions) {
            for (auto permission : *parsed.permissions) {
                existing->grant_permission(domain::Permission::create(permission).value());
            }
        }

        auto result = repository.update(*existing);
        if (!result) {
            co_await uow->rollback();
            std::vector<std::string> changes;
            if (parsed.password) {
                changes.emplace_back("password update");
            }
            if (parsed.permissions && !parsed.permissions->empty()) {
                changes.push_back(
                    fmt::format("permissions [{}]", fmt::join(*parsed.permissions, ", ")));
            }

            auto error_msg = fmt::format("Failed to update user '{}' (ID: {}) with changes: [{}]",
                                         existing->name(), user_id, fmt::join(changes, ", "));

            co_return make_error_response(error_msg, drogon::k422UnprocessableEntity);
        }

        co_await uow->commit();

        co_return make_json_response(utils::convert_to<UserResponse, domain::User> {}(*result));
    } else {
        co_return make_error_response(
            fmt::format("Database transaction initialization failed for updating user ID '{}'",
                        user_id),
            drogon::k503ServiceUnavailable);
    }
}

ResponseTask UserController::delete_user([[maybe_unused]] drogon::HttpRequestPtr req,

                                         std::string user_id) {
    if (auto [uow, err, repository] = co_await m_uow_factory.with_rw_repositories<domain::User>();
        !err) {
        // Check if user exists before deletion for better error messaging
        auto existing_user = repository.get_by_id(user_id);
        if (!existing_user) {
            co_return make_error_response(fmt::format("User with ID '{}' does not exist", user_id),
                                          drogon::k404NotFound);
        }

        auto result = repository.delete_by_id(user_id);
        if (!result) {
            co_await uow->rollback();
            co_return make_error_response(
                fmt::format("Cannot delete user '{}' (ID: {}). User may be referenced by other "
                            "entities or have active sessions",
                            existing_user->name(), user_id),
                drogon::k409Conflict);
        }

        co_await uow->commit();
        co_return drogon_helpers::make_json_response(user_id);
    } else {
        co_return make_error_response(
            fmt::format("Database transaction initialization failed for deleting user ID '{}'",
                        user_id),
            drogon::k503ServiceUnavailable);
    }
}

ResponseTask UserController::get_users(drogon::HttpRequestPtr req) {
    static constexpr drogon_helpers::PaginationLimits pagination_limits {
        .MinPage = 1,
        .DefaultLimit = 10,
        .MinLimit = 1,
        .MaxLimit = 100,
    };

    auto const [page, limit, offset] = drogon_helpers::get_pagination<pagination_limits>(req);

    // Validate pagination parameters
    if (page < pagination_limits.MinPage) {
        co_return make_error_response(
            fmt::format("Invalid page number {}. Must be >= {}", page, pagination_limits.MinPage),
            drogon::k400BadRequest);
    }

    if (limit < pagination_limits.MinLimit || limit > pagination_limits.MaxLimit) {
        co_return make_error_response(fmt::format("Invalid limit {}. Must be between {} and {}",
                                                  limit, pagination_limits.MinLimit,
                                                  pagination_limits.MaxLimit),
                                      drogon::k400BadRequest);
    }

    if (auto [uow, err, repository] = co_await m_uow_factory.with_ro_repositories<domain::User>();
        !err) {
        auto users = repository.all();

        if (!users) {
            co_await uow->rollback();
            co_return make_error_response(fmt::format("Database query failed while retrieving "
                                                      "users (page: {}, limit: {}, offset: {})",
                                                      page, limit, offset),
                                          drogon::k500InternalServerError);
        }

        co_return std::move(*users) | std::views::drop(offset) | std::views::take(limit)
            | views::convert<UserResponse> | adapters::drogon::to_json_response;

    } else {
        co_return make_error_response(
            fmt::format(
                "Database connection unavailable for users list query (page: {}, limit: {})", page,
                limit),
            drogon::k503ServiceUnavailable);
    }
}

ResponseTask UserController::get_user([[maybe_unused]] drogon::HttpRequestPtr req,
                                      std::string user_id) {
    if (auto [uow, err, repository] = co_await m_uow_factory.with_ro_repositories<domain::User>();
        !err) {
        auto user = repository.get_by_id(user_id);
        if (!user) {
            co_await uow->rollback();
            co_return make_error_response(fmt::format("User with ID '{}' not found", user_id),
                                          drogon::k404NotFound);
        }

        co_await uow->commit();

        co_return make_json_response(utils::convert_to<UserResponse, domain::User> {}(*user));
    }

    co_return make_error_response(
        fmt::format("Database connection unavailable for retrieving user ID '{}'", user_id),
        drogon::k503ServiceUnavailable);
}
} // namespace bxt::Users
