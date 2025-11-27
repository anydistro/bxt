/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "LogMiddleware.hpp"

#include <drogon/HttpTypes.h>

#include "infra/logging/Logging.hpp"

namespace bxt::middleware {

void LogMiddleware::invoke(HttpRequestPtr const& req,
                           MiddlewareNextCallback&& nextCb,
                           MiddlewareCallback&& mcb) {
    // Log incoming request
    if (auto json = req->getJsonObject(); json) {
        bxt::logi_s("🌐📥 New request from {}: {}", json->toStyledString(),
                    req->getPeerAddr().toIpPort(), req->getPath());
    } else {
        bxt::logi("🌐📥 New request from {}: {}", req->getPeerAddr().toIpPort(), req->getPath());
    }

    // Call next middleware and get response
    nextCb([req, mcb](auto&& res) {
        auto status = static_cast<int>(res->statusCode());
        auto peer = req->getPeerAddr().toIpPort();
        constexpr auto log_fmt = "🌐📤 {} response to {} with status {}";

        // Helper lambda template that handles both regular and structured logging
        auto logi_response = [&](std::string_view prefix) {
            if (auto json = res->getJsonObject(); json) {
                bxt::logi_s(log_fmt, json->toStyledString(), prefix, peer, status);
            } else {
                bxt::logi(log_fmt, prefix, peer, status);
            }
        };

        auto logw_response = [&](std::string_view prefix) {
            if (auto json = res->getJsonObject(); json) {
                bxt::logw_s(log_fmt, json->toStyledString(), prefix, peer, status);
            } else {
                bxt::logw(log_fmt, prefix, peer, status);
            }
        };

        // Using the lambda with different log functions based on status
        if (status >= drogon::k500InternalServerError) {
            // Server errors as warnings
            logw_response("Server error");
        } else if (status == drogon::k401Unauthorized || status == drogon::k403Forbidden) {
            // Auth failures - still as info but with distinct prefix
            logi_response("Auth failure");
        } else if (status >= drogon::k400BadRequest) {
            // Regular client errors as info
            logi_response("Client error");
        } else {
            // Success responses
            logi_response("Success");
        }

        mcb(res);
    });
}
} // namespace bxt::middleware
