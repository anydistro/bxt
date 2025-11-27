/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <expected>
#include <ranges>

#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <json/value.h>
#include <rfl/json/write.hpp>
#include <rfl/SnakeCaseToCamelCase.hpp>

namespace bxt::adapters::drogon {
struct to_json_response_t {
    template<std::ranges::range R>

    constexpr auto operator()(R&& range,

                              ::drogon::HttpStatusCode success_code = ::drogon::k200OK,
                              bool raw = false,
                              bool fail_on_error = true) const {
        using ValueType =
            std::remove_cvref_t<decltype(std::declval<std::ranges::range_value_t<R>>().value())>;
        using ::drogon::HttpResponse;
        Json::Value json_response(Json::objectValue);
        Json::Value data_array(Json::arrayValue);
        Json::Value errors_array(Json::arrayValue);

        size_t total_count = 0;
        size_t success_count = 0;

        auto json_write =
            raw ? +[](ValueType&
                          value) { return rfl::json::write(std::forward<decltype(value)>(value)); }
                : +[](ValueType& value) {
                      return rfl::json::write<rfl::SnakeCaseToCamelCase>(
                          std::forward<decltype(value)>(value));
                  };

        // Process each item
        for (auto&& result : range) {
            total_count++;

            if (!result.has_value()) {
                // Collect errors instead of failing immediately
                Json::Value error(Json::objectValue);
                error["message"] = result.error().what();
                error["index"] = static_cast<int>(total_count - 1);
                errors_array.append(error);

                if (fail_on_error) {
                    // Return structured error response
                    Json::Value error_response(Json::objectValue);
                    error_response["error"] = result.error().what();
                    error_response["status"] = "error";

                    auto response = HttpResponse::newHttpJsonResponse(error_response);
                    response->setStatusCode(::drogon::k400BadRequest);
                    return response;
                }
            } else {
                success_count++;
                // Add successful items to data array
                data_array.append(json_write(result.value()));
            }
        }

        // Build response with metadata
        json_response["data"] = data_array;
        json_response["meta"] = Json::Value(Json::objectValue);
        json_response["meta"]["total"] = static_cast<int>(total_count);
        json_response["meta"]["success"] = static_cast<int>(success_count);

        if (!errors_array.empty()) {
            json_response["errors"] = errors_array;
        }

        // Better status code logic
        ::drogon::HttpStatusCode response_code = ::drogon::k200OK;
        if (total_count == 0) {
            response_code = success_code;
        } else if (success_count == total_count) {
            response_code = success_code;
        } else if (success_count == 0) {
            response_code = ::drogon::k400BadRequest;
        } else {
            response_code = ::drogon::k207MultiStatus;
        }

        auto response = ::drogon::HttpResponse::newHttpJsonResponse(json_response);
        response->setStatusCode(response_code);
        return response;
    }
};

template<std::ranges::viewable_range R>
constexpr auto operator|(R&& range, to_json_response_t const& adapter) {
    return adapter(std::forward<R>(range));
}

inline constexpr to_json_response_t to_json_response {};
} // namespace bxt::adapters::drogon
