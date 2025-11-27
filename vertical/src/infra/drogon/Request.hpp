/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include <expected>
#include <string>

#include <drogon/HttpRequest.h>
#include <fmt/format.h>

#include "Helpers.hpp"

namespace bxt {

template<typename T> struct ParsedRequest {
    ParsedRequest() = default;

    static ParsedRequest<T> from_request(drogon::HttpRequestPtr req) {
        ParsedRequest<T> parsed;
        auto result = drogon_helpers::get_request_json<T>(req);

        if (!result) {
            parsed.request = std::unexpected(result.error().what());
            return parsed;
        }

        parsed.request = std::move(result.value());
        //     parsed.request = std::unexpected(result.error().what());
        // }

        // parsed.request = std::move(result.value());

        // auto attrs = req->attributes();
        // if (!attrs->find(fmt::format("jwt_{}", Presentation::Names::UserName))) {
        //     return parsed;
        // }

        // auto username_any = (*attrs)[fmt::format("jwt_{}", Presentation::Names::UserName)];
        // if (username_any.type() != typeid(std::string)) {
        //     return parsed;
        // }

        // parsed.username = std::any_cast<std::string>(username_any);

        return parsed;
    }

    std::expected<T, std::string> request;
    std::optional<std::string> username;
};

} // namespace bxt
