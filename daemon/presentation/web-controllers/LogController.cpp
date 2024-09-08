/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "LogController.h"

#include "presentation/messages/LogMessages.h"
#include "utilities/drogon/Helpers.h"
#include "utilities/reflect/TimePointParser.h"

#include <optional>
#include <rfl.hpp>

namespace bxt::Presentation {
drogon::Task<drogon::HttpResponsePtr> LogController::get_package_logs(drogon::HttpRequestPtr req,
                                                                      std::string const& from_str,
                                                                      std::string const& to_str,
                                                                      std::string const& text) {
    BXT_JWT_CHECK_PERMISSIONS("logs", req)

    if (from_str.empty() || to_str.empty()) {
        co_return drogon_helpers::make_error_response("Missing 'since' or 'until' parameters");
    }

    auto const from_result = Iso8601TimePoint::from_string(from_str);
    if (!from_result) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Invalid 'since' time format: {}", from_result.error()->what()));
    }

    auto const from = (*from_result).to_class();

    auto const to_result = Iso8601TimePoint::from_string(to_str);
    if (!to_result) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Invalid 'until' time format: {}", to_result.error()->what()));
    }
    auto const to = (*to_result).to_class();

    auto const dto = co_await m_service.events(
        {.since = from,
         .until = to,
         .full_text = text.empty() ? std::nullopt : std::make_optional(text)});

    if (dto.commits.empty() && dto.syncs.empty() && dto.deploys.empty()) {
        drogon_helpers::make_json_response(LogResponse {});
    }

    static constexpr auto package_mapping = [](auto const& dto) {
        PackageLogEntryResponse response;
        response.type = dto.type;
        response.section = dto.section;
        response.name = dto.name;
        response.location = bxt::to_string(dto.location);
        response.version = dto.version;
        return response;
    };
    co_return drogon_helpers::make_json_response(LogResponse {
        .syncs = Utilities::map_entries(
            dto.syncs, ([](auto const& dto) {
                return SyncLogEntryResponse {dto.time, dto.sync_trigger_username,
                                             Utilities::map_entries(dto.added, package_mapping),
                                             Utilities::map_entries(dto.deleted, package_mapping)};
            })),
        .commits =
            Utilities::map_entries(dto.commits, ([](auto const& dto) {
                                       return CommitLogEntryResponse {
                                           dto.time, dto.commiter_username,
                                           Utilities::map_entries(dto.added, package_mapping),
                                           Utilities::map_entries(dto.deleted, package_mapping)};
                                   })),
        .deploys = Utilities::map_entries(
            dto.deploys, ([](auto const& dto) {
                return DeployLogEntryResponse {dto.time, dto.runner_url,
                                               Utilities::map_entries(dto.added, package_mapping)};
            }))});
}

} // namespace bxt::Presentation
