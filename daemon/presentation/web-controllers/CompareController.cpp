/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "CompareController.h"

#include "core/application/dtos/PackageSectionDTO.h"
#include "drogon/HttpResponse.h"
#include "drogon/HttpTypes.h"
#include "presentation/messages/CompareMessages.h"
#include "utilities/drogon/Helpers.h"
#include "utilities/drogon/Macro.h"
#include "utilities/to_string.h"

#include <rfl/as.hpp>
#include <rfl/json/read.hpp>
#include <string>
#include <unordered_map>
#include <vector>
namespace bxt::Presentation {
drogon::Task<drogon::HttpResponsePtr>
    CompareController::compare(drogon::HttpRequestPtr req) {
    const auto available_sections = co_await m_section_service.get_sections();

    for (const auto& [branch, repository, architecture] : *available_sections) {
        BXT_JWT_CHECK_PERMISSIONS(fmt::format("packages.compare.{}.{}.{}",
                                              branch, repository, architecture),
                                  req)
    }

    if (!available_sections.has_value()) {
        co_return drogon_helpers::make_error_response("No sections available");
    }

    const auto sections_request =
        drogon_helpers::get_request_json<CompareRequest>(req);

    if (!sections_request) {
        co_return drogon_helpers::make_error_response(fmt::format(
            "Invalid request: {}", sections_request.error()->what()));
    }

    if ((*sections_request).empty()) {
        co_return drogon_helpers::make_error_response(
            "No sections to compare were provided");
    }

    const auto compare_result =
        co_await m_compare_service.compare(*sections_request);

    if (compare_result->sections.empty()
        || compare_result->compare_table.empty()) {
        co_return drogon_helpers::make_error_response(
            "No compare data found (all sections are empty)");
    }

    CompareResponse result {compare_result->sections};

    for (const auto& [index, version] : compare_result->compare_table) {
        auto&& [name, section, location] = index;

        result.compare_table[name][bxt::to_string(section)]
                            [bxt::to_string(location)] = version;
    }

    co_return drogon_helpers::make_json_response(result);
}
} // namespace bxt::Presentation
