/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

namespace bxt::Monitoring {

class MetricsController : public drogon::HttpController<MetricsController, false> {
public:
    MetricsController() = default;
    METHOD_LIST_BEGIN
    METHOD_ADD(MetricsController::get_metrics, "/metrics", drogon::Get);
    METHOD_LIST_END

    drogon::Task<drogon::HttpResponsePtr> get_metrics(drogon::HttpRequestPtr req);
};

} // namespace bxt::Monitoring
