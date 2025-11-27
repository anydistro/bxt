/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <drogon/HttpMiddleware.h>
#include <drogon/HttpRequest.h>

#include "infra/logging/StructuredLogging.hpp"

namespace bxt::middleware {
class LogMiddleware : public drogon::HttpMiddleware<LogMiddleware, false> {
    using HttpRequestPtr = drogon::HttpRequestPtr;
    using MiddlewareNextCallback = drogon::MiddlewareNextCallback;
    using MiddlewareCallback = drogon::MiddlewareCallback;

public:
    explicit LogMiddleware() = default;

    void invoke(HttpRequestPtr const& req,
                MiddlewareNextCallback&& nextCb,
                MiddlewareCallback&& mcb) override;
};
} // namespace bxt::middleware
