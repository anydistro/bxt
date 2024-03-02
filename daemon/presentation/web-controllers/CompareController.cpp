/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "CompareController.h"

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/enums/PoolLocation.h"
#include "drogon/HttpResponse.h"
#include "drogon/HttpTypes.h"
#include "utilities/drogon/Macro.h"

#include "json/value.h"
#include <string>
#include <vector>
namespace bxt::Presentation {
drogon::Task<drogon::HttpResponsePtr>
    CompareController::compare(drogon::HttpRequestPtr req) {
    Json::Value result;
    const auto available_sections = co_await m_section_service.get_sections();

    if (!available_sections.has_value()) {
        result["error"] = "No sections available in the Box";
        result["status"] = "error";

        auto response = drogon::HttpResponse::newHttpJsonResponse(result);
        response->setStatusCode(drogon::k400BadRequest);

        co_return response;
    }

    for (const auto& [branch, repository, architecture] : *available_sections) {
        BXT_JWT_CHECK_PERMISSIONS(fmt::format("packages.compare.{}.{}.{}",
                                              branch, repository, architecture),
                                  req)
    }

    const auto sections_json = *req->getJsonObject();

    if (sections_json.empty()) {
        result["error"] = "No sections to compare provided";
        result["status"] = "error";

        auto response = drogon::HttpResponse::newHttpJsonResponse(result);
        response->setStatusCode(drogon::k400BadRequest);

        co_return response;
    }

    std::vector<PackageSectionDTO> sections;

    for (const auto& section_json : sections_json) {
        sections.emplace_back(PackageSectionDTO {
            .branch = section_json["branch"].asString(),
            .repository = section_json["repository"].asString(),
            .architecture = section_json["architecture"].asString()});
    }

    const auto compare_result = co_await m_compare_service.compare(sections);

    if (compare_result->sections.empty()
        || compare_result->compare_table.empty()) {
        result["error"] = "No compare data found (all sections are empty)";
        result["status"] = "error";

        auto response = drogon::HttpResponse::newHttpJsonResponse(result);
        response->setStatusCode(drogon::k400BadRequest);

        co_return response;
    }

    for (const auto& section : compare_result->sections) {
        Json::Value section_json;

        section_json["branch"] = section.branch;
        section_json["repository"] = section.repository;
        section_json["architecture"] = section.architecture;

        result["sections"].append(section_json);
    }

    for (const auto& [index, version] : compare_result->compare_table) {
        auto&& [name, section, location] = index;
        result["compare_table"][name][std::string(section)]
              [bxt::to_string(location)] = version;
    }

    co_return drogon::HttpResponse::newHttpJsonResponse(result);
}
} // namespace bxt::Presentation
