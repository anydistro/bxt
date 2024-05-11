/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "SectionController.h"

#include "presentation/messages/SectionMessages.h"
#include "utilities/drogon/Helpers.h"

#include <drogon/HttpResponse.h>
#include <json/value.h>
#include <ranges>

drogon::Task<drogon::HttpResponsePtr>
    bxt::Presentation::SectionController::get_sections(
        drogon::HttpRequestPtr req) const {
    auto sections = co_await m_service.get_sections();

    if (!sections.has_value()) {
        co_return drogon_helpers::make_error_response(sections.error().what());
    }

    sections = *sections | std::views::filter([this, req](auto section) {
        return coro::sync_wait(m_permission_service.check(
            fmt::format("sections.{}.{}.{}", section.branch, section.repository,
                        section.architecture),
            req->getAttributes()->get<std::string>("jwt_username")));
    }) | std::ranges::to<std::vector>();

    co_return drogon_helpers::make_json_response(*sections);
}
