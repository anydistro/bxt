/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "MaintenanceController.h"

#include "core/application/dtos/PackageSectionDTO.h"
#include "presentation/messages/MaintenanceMessages.h"
#include "utilities/drogon/Helpers.h"
#include "utilities/drogon/Macro.h"
namespace bxt::Presentation {

using namespace drogon;

Task<HttpResponsePtr> MaintenanceController::export_database(HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("maintenance.export", req);

    auto request = drogon_helpers::get_request_json<ExportDatabaseRequest>(req);

    co_await m_service.export_database(
        request.value_or({{}}).sections | std::views::transform([](SectionRequest const& section) {
            return PackageSectionDTO {.branch = section.branch,
                                      .repository = section.repository,
                                      .architecture = section.architecture};
        })
        | std::ranges::to<std::set>());

    co_return {};
}

} // namespace bxt::Presentation
