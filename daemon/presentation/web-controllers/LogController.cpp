/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "LogController.h"

#include "core/application/dtos/PackageLogEntryDTO.h"
#include "core/domain/enums/LogEntryType.h"
#include "core/domain/enums/PoolLocation.h"
#include "drogon/HttpTypes.h"
#include "presentation/messages/LogMessages.h"
#include "presentation/messages/PackageMessages.h"
#include "utilities/drogon/Helpers.h"
#include "utilities/log/Logging.h"
#include "utilities/reflect/PathParser.h"

#include <filesystem>
#include <rfl.hpp>

namespace bxt::Presentation {
drogon::Task<drogon::HttpResponsePtr>
    LogController::get_package_logs(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("logs", req)

    const auto dtos = co_await m_service.events();

    if (dtos.empty()) {
        co_return drogon_helpers::make_error_response("No events found");
    }

    constexpr auto package_mapping = [](const auto& e) {
        auto&& [location, entry] = e;
        return std::make_pair(
            bxt::to_string(location),
            PoolEntryResponse {entry.version,
                               entry.signature_path.has_value()});
    };

    co_return drogon_helpers::make_json_response(
        dtos
        | std::views::transform(
            [package_mapping](const PackageLogEntryDTO& dto) {
                auto&& [id, time, type, package] = dto;
                LogEntryReponse result {
                    id, time, type,
                    PackageResponse {
                        package.name, package.section,
                        package.pool_entries
                            | std::views::transform(package_mapping)
                            | std::ranges::to<std::unordered_map>()}};

                if (const auto preferred_location =
                        Core::Domain::select_preferred_pool_location(
                            package.pool_entries)) {
                    result.package.preferred_location =
                        bxt::to_string(*preferred_location);
                } else {
                    logd("Package {} has no pool entries, skipping preferred "
                         "one selection",
                         package.name);
                }
                return result;
            })
        | std::ranges::to<std::vector>());
}

} // namespace bxt::Presentation
