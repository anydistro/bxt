/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PackageController.h"

#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/services/PackageService.h"
#include "core/domain/enums/PoolLocation.h"
#include "presentation/messages/PackageMessages.h"
#include "presentation/Names.h"
#include "utilities/drogon/Helpers.h"
#include "utilities/drogon/Macro.h"
#include "utilities/log/Logging.h"
#include "utilities/to_string.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <drogon/HttpResponse.h>
#include <drogon/MultiPart.h>
#include <drogon/utils/FunctionTraits.h>
#include <json/value.h>
#include <map>
#include <ranges>
#include <rfl/as.hpp>
#include <rfl/json/read.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace bxt::Presentation {
using namespace drogon;

drogon::Task<HttpResponsePtr> PackageController::sync(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("packages.sync", req)
    drogon::async_run([this, req]() -> drogon::Task<void> {
        co_await m_sync_service.sync_all({.user_name = req->attributes()->get<std::string>(
                                              fmt::format("jwt_{}", Names::UserName))});
        co_return;
    });

    co_return HttpResponse::newHttpResponse();
}
drogon::Task<drogon::HttpResponsePtr>
    PackageController::commit_transaction(drogon::HttpRequestPtr req) {
    MultiPartParser file_upload;
    file_upload.parse(req);

    auto const files_map = file_upload.getFilesMap();
    auto const params_map = file_upload.getParameters();

    std::map<int, PackageDTO> packages;

    for (auto const& [name, file] : files_map) {
        if (!name.starts_with("package")) {
            continue;
        }
        std::vector<std::string> parts;
        boost::split(parts, name, boost::is_any_of("."));

        if (parts.size() > 3) {
            continue;
        }

        auto file_number_str = parts[0].substr(7);
        auto const file_number = std::stoi(file_number_str);

        if (!packages.contains(file_number)) {
            packages.emplace(file_number, PackageDTO {});
        }

        file.save();

        auto location = Core::Domain::PoolLocation::Overlay;

        PackagePoolEntryDTO& pool_entry = packages[file_number].pool_entries[location];

        if (parts.size() == 1 || parts[1] != "signature") {
            pool_entry.filepath = app().getUploadPath() + "/" + file.getFileName();

        } else if (parts[1] == "signature") {
            pool_entry.signature_path = app().getUploadPath() + "/" + file.getFileName();
        }
    }

    for (auto const& [name, param] : params_map) {
        if (!name.starts_with("package")) {
            continue;
        }

        std::vector<std::string> parts;
        boost::split(parts, name, boost::is_any_of("."));

        if (parts.size() != 2) {
            continue;
        }

        auto file_number_str = parts[0].substr(7);
        auto const file_number = std::stoi(file_number_str);

        if (!packages.contains(file_number)) {
            continue;
        }

        if (parts[1] == "section") {
            auto section = rfl::json::read<PackageSectionDTO, rfl::SnakeCaseToCamelCase>(param);

            if (!section) {
                co_return drogon_helpers::make_error_response(
                    fmt::format("Invalid section format: {}", section.error()->what()));
            }
            packages[file_number].section = *section;
        }
    }

    PackageService::Transaction transaction;
    for (auto& package : packages) {
        auto const& [branch, repository, architecture] = package.second.section;

        BXT_JWT_CHECK_PERMISSIONS(
            (std::vector<std::string_view> {
                fmt::format("packages.commit.{}.{}.{}", branch, repository, architecture),
                fmt::format("sections.{}.{}.{}", branch, repository, architecture)}),
            req)

        transaction.to_add.emplace_back(std::move(package.second));
    }

    auto to_delete_it = params_map.find("to_delete");
    if (to_delete_it != params_map.end()) {
        auto to_delete = rfl::json::read<std::vector<PackageService::Transaction::PackageAction>>(
            to_delete_it->second);

        if (!to_delete) {
            co_return drogon_helpers::make_error_response(
                fmt::format("Invalid to_delete format: {}", to_delete.error()->what()));
        }

        for (auto const& action : *to_delete) {
            BXT_JWT_CHECK_PERMISSIONS(
                (std::vector<std::string_view> {
                    fmt::format("packages.commit.{}.{}.{}", action.section.branch,
                                action.section.repository, action.section.architecture),
                    fmt::format("sections.{}.{}.{}", action.section.branch,
                                action.section.repository, action.section.architecture)}),
                req)
            transaction.to_delete.emplace_back(action);
        }
    }

    auto to_move_it = params_map.find("to_move");
    if (to_move_it != params_map.end()) {
        auto to_move = rfl::json::read<std::vector<PackageService::Transaction::TransferAction>>(
            to_move_it->second);

        if (!to_move) {
            co_return drogon_helpers::make_error_response(
                fmt::format("Invalid to_move format: {}", to_move.error()->what()));
        }

        for (auto const& action : *to_move) {
            BXT_JWT_CHECK_PERMISSIONS(
                (std::vector<std::string_view> {
                    fmt::format("packages.commit.{}.{}.{}", action.to_section.branch,
                                action.to_section.repository, action.to_section.architecture),
                    fmt::format("sections.{}.{}.{}", action.from_section.branch,
                                action.from_section.repository, action.from_section.architecture),
                    fmt::format("sections.{}.{}.{}", action.to_section.branch,
                                action.to_section.repository, action.to_section.architecture)}),
                req)
            transaction.to_move.emplace_back(action);
        }
    }
    auto to_copy_it = params_map.find("to_copy");
    if (to_copy_it != params_map.end()) {
        auto to_copy = rfl::json::read<std::vector<PackageService::Transaction::TransferAction>>(
            to_copy_it->second);

        if (!to_copy) {
            co_return drogon_helpers::make_error_response(
                fmt::format("Invalid to_copy format: {}", to_copy.error()->what()));
        }

        for (auto const& action : *to_copy) {
            BXT_JWT_CHECK_PERMISSIONS(
                (std::vector<std::string_view> {
                    fmt::format("packages.commit.{}.{}.{}", action.to_section.branch,
                                action.to_section.repository, action.to_section.architecture),
                    fmt::format("sections.{}.{}.{}", action.from_section.branch,
                                action.from_section.repository, action.from_section.architecture),
                    fmt::format("sections.{}.{}.{}", action.to_section.branch,
                                action.to_section.repository, action.to_section.architecture)}),
                req)
            transaction.to_copy.emplace_back(action);
        }
    }

    auto result = co_await m_package_service.push(
        transaction,
        {.user_name = req->attributes()->get<std::string>(fmt::format("jwt_{}", Names::UserName))});

    if (!result.has_value()) {
        co_return drogon_helpers::make_error_response(result.error().what());
    }
    co_return drogon_helpers::make_ok_response();
}

drogon::Task<drogon::HttpResponsePtr>
    PackageController::get_packages(drogon::HttpRequestPtr req,
                                    std::string const& branch,
                                    std::string const& repository,
                                    std::string const& architecture) {
    if (branch.empty() || repository.empty() || architecture.empty()) {
        co_return drogon_helpers::make_error_response(
            "Branch, repository and architecture must be specified");
    }
    PackageSectionDTO section {branch, repository, architecture};

    BXT_JWT_CHECK_PERMISSIONS(
        (std::vector<std::string_view> {
            fmt::format("packages.get.{}.{}.{}", branch, repository, architecture),
            fmt::format("sections.{}.{}.{}", branch, repository, architecture)}),
        req)

    auto const packages = co_await m_package_service.get_packages(section);

    if (!packages) {
        co_return drogon_helpers::make_error_response(packages.error().what());
    }

    {
        using std::ranges::to;
        using std::views::transform;
        auto response = *packages | transform([](auto const& dto) {
            auto&& [section, name, is_any_architecture, pool_entries] = dto;

            PackageResponse result {
                name, section,
                pool_entries | transform([](auto const& e) {
                    auto&& [location, entry] = e;

                    return std::make_pair(
                        bxt::to_string(location),
                        PoolEntryResponse {entry.version, entry.signature_path.has_value()});
                }) | to<std::unordered_map>()};

            if (auto const preferred_location =
                    Core::Domain::select_preferred_pool_location(pool_entries)) {
                result.preferred_location = bxt::to_string(*preferred_location);
            } else {
                logd("Package {} has no pool entries, skipping preferred "
                     "one selection",
                     dto.name);
            }

            return result;
        }) | to<std::vector>();

        co_return drogon_helpers::make_json_response(response);
    }
}

drogon::Task<drogon::HttpResponsePtr> PackageController::snap(drogon::HttpRequestPtr req) {
    auto const snap_request = rfl::json::read<SnapRequest>(std::string(req->getBody()));

    if (snap_request.error()) {
        co_return drogon_helpers::make_error_response("Invalid arguments");
    }

    auto& source_branch = (*snap_request).source;

    BXT_JWT_CHECK_PERMISSIONS(
        (std::vector<std::string_view> {
            fmt::format("advanced.packages.snap.{}.{}.{}", source_branch.branch,
                        source_branch.repository, source_branch.architecture),
            fmt::format("sections.{}.{}.{}", source_branch.branch, source_branch.repository,
                        source_branch.architecture)}),
        req)

    auto& target_branch = (*snap_request).target;

    BXT_JWT_CHECK_PERMISSIONS(
        (std::vector<std::string_view> {
            fmt::format("packages.get.{}.{}.{}", target_branch.branch, target_branch.repository,
                        target_branch.architecture),
            fmt::format("sections.{}.{}.{}", target_branch.branch, target_branch.repository,
                        target_branch.architecture)}),
        req)

    auto const snap_ok = co_await m_package_service.snap(source_branch, target_branch);

    if (!snap_ok.has_value()) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Snap failed: {}", snap_ok.error().what()));
    }

    co_return drogon_helpers::make_ok_response();
}

drogon::Task<drogon::HttpResponsePtr> PackageController::snap_branch(drogon::HttpRequestPtr req) {
    auto const snap_request = rfl::json::read<SnapBranchRequest>(std::string(req->getBody()));

    if (snap_request.error()) {
        co_return drogon_helpers::make_error_response("Invalid arguments");
    }
    auto& source_branch = (*snap_request).source_branch;
    auto& target_branch = (*snap_request).target_branch;
    auto& arch = (*snap_request).architecture;

    BXT_JWT_CHECK_PERMISSIONS(
        (std::vector<std::string_view> {fmt::format("sections.{}.*.{}", source_branch, arch),
                                        fmt::format("packages.snap.from.{}", source_branch),
                                        fmt::format("sections.{}.*.{}", target_branch, arch),
                                        fmt::format("packages.snap.to.{}", target_branch)}),
        req)

    auto const snap_ok = co_await m_package_service.snap_branch(source_branch, target_branch, arch);

    if (!snap_ok.has_value()) {
        co_return drogon_helpers::make_error_response(
            fmt::format("Snap section failed: {}", snap_ok.error().what()));
    }

    co_return drogon_helpers::make_ok_response();
}
} // namespace bxt::Presentation
