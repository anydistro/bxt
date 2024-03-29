/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "SectionController.h"

#include <drogon/HttpResponse.h>
#include <json/value.h>

drogon::Task<drogon::HttpResponsePtr>
    bxt::Presentation::SectionController::get_sections(
        drogon::HttpRequestPtr req) const {
    const auto sections = co_await m_service.get_sections();

    Json::Value result;

    if (!sections.has_value()) {
        result["status"] = "error";
        result["message"] = sections.error().what();

        co_return drogon::HttpResponse::newHttpJsonResponse(result);
    }

    for (const auto& [branch, repository, architecture] : *sections) {
        if (!co_await m_permission_service.check(
                fmt::format("sections.{}.{}.{}", branch, repository,
                            architecture),
                req->getAttributes()->get<std::string>("jwt_username"))) {
            continue;
        }

        Json::Value section_json;

        section_json["branch"] = branch;
        section_json["repository"] = repository;
        section_json["architecture"] = architecture;

        result.append(section_json);
    }

    co_return drogon::HttpResponse::newHttpJsonResponse(result);
}
