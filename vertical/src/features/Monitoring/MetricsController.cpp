/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "MetricsController.hpp"

namespace bxt::Monitoring {

drogon::Task<drogon::HttpResponsePtr>
    MetricsController::get_metrics([[maybe_unused]] drogon::HttpRequestPtr req) {
    co_return drogon::HttpResponse::newHttpResponse();
}

} // namespace bxt::Monitoring
