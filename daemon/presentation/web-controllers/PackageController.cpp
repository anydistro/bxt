/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PackageController.h"

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/split.hpp"
#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/services/PackageService.h"
#include "core/domain/enums/PoolLocation.h"
#include "core/domain/value_objects/PackageVersion.h"
#include "drogon/HttpResponse.h"
#include "drogon/HttpTypes.h"
#include "drogon/utils/FunctionTraits.h"
#include "jwt-cpp/traits/nlohmann-json/defaults.h"
#include "utilities/drogon/Macro.h"
#include "utilities/log/Logging.h"

#include "json/value.h"
#include <drogon/MultiPart.h>
#include <map>
#include <string>
#include <vector>

namespace bxt::Presentation {
using namespace drogon;

drogon::Task<HttpResponsePtr>
    PackageController::sync(drogon::HttpRequestPtr req) {
    BXT_JWT_CHECK_PERMISSIONS("packages.sync", req)

    co_await m_sync_service.sync_all();

    co_return HttpResponse::newHttpResponse();
}
drogon::Task<drogon::HttpResponsePtr>
    PackageController::commit_transaction(drogon::HttpRequestPtr req) {
    MultiPartParser file_upload;
    file_upload.parse(req);

    const auto files_map = file_upload.getFilesMap();
    const auto params_map = file_upload.getParameters();

    std::map<int, PackageDTO> packages;

    for (const auto &[name, file] : files_map) {
        if (!name.starts_with("package")) continue;
        std::vector<std::string> parts;
        boost::split(parts, name, boost::is_any_of("."));

        if (parts.size() > 3) continue;

        auto file_number_str = parts[0].substr(7);
        const auto file_number = std::stoi(file_number_str);

        if (!packages.contains(file_number)) {
            packages.emplace(file_number, PackageDTO {});
        }

        file.save();

        auto location = Core::Domain::PoolLocation::Overlay;

        PackagePoolEntryDTO &pool_entry =
            packages[file_number].pool_entries[location];

        if (parts.size() == 1 || parts[1] != "signature") {
            pool_entry.filepath =
                app().getUploadPath() + "/" + file.getFileName();

        } else if (parts[1] == "signature") {
            pool_entry.signature_path =
                app().getUploadPath() + "/" + file.getFileName();
        }
    }

    for (const auto &[name, param] : params_map) {
        if (!name.starts_with("package")) continue;

        std::vector<std::string> parts;
        boost::split(parts, name, boost::is_any_of("."));

        if (parts.size() != 2) continue;

        auto file_number_str = parts[0].substr(7);
        const auto file_number = std::stoi(file_number_str);

        if (!packages.contains(file_number)) continue;

        if (parts[1] == "branch") {
            packages[file_number].section.branch = param;
        } else if (parts[1] == "repository") {
            packages[file_number].section.repository = param;
        } else if (parts[1] == "architecture") {
            packages[file_number].section.architecture = param;
        }
    }

    PackageService::Transaction transaction;
    for (auto &package : packages) {
        const auto &[branch, repository, architecture] = package.second.section;

        BXT_JWT_CHECK_PERMISSIONS(
            (std::vector<std::string_view> {
                fmt::format("packages.commit.{}.{}.{}", branch, repository,
                            architecture),
                fmt::format("sections.{}.{}.{}", branch, repository,
                            architecture)}),
            req)

        transaction.to_add.emplace_back(std::move(package.second));
    }

    auto result = co_await m_package_service.commit_transaction(transaction);

    Json::Value res_json;
    res_json["result"] = result ? "ok" : "error";

    auto response = drogon::HttpResponse::newHttpJsonResponse(res_json);

    co_return response;
}

drogon::Task<drogon::HttpResponsePtr>
    PackageController::get_packages(drogon::HttpRequestPtr req,
                                    const std::string &branch,
                                    const std::string &repository,
                                    const std::string &architecture) {
    PackageSectionDTO section {branch, repository, architecture};
    Json::Value result;

    BXT_JWT_CHECK_PERMISSIONS((std::vector<std::string_view> {
                                  fmt::format("packages.get.{}.{}.{}", branch,
                                              repository, architecture),
                                  fmt::format("sections.{}.{}.{}", branch,
                                              repository, architecture)}),
                              req)

    const auto packages = co_await m_package_service.get_packages(section);

    if (!packages.has_value()) {
        result["status"] = "error";
        result["message"] = packages.error().what();
        co_return drogon::HttpResponse::newHttpJsonResponse(result);
    }

    for (const auto &package : *packages) {
        Json::Value package_json;

        package_json["name"] = package.name;
        package_json["section"] = Json::Value();
        package_json["section"]["branch"] = package.section.branch;
        package_json["section"]["repository"] = package.section.repository;
        package_json["section"]["architecture"] = package.section.architecture;

        Json::Value pool_entries_json;
        for (const auto &[pool_location, pool_entry] : package.pool_entries) {
            Json::Value entry_json;
            entry_json["version"] = pool_entry.version;
            entry_json["hasSignature"] =
                pool_entry.signature_path.has_value() ? "true" : "false";

            pool_entries_json
                [Core::Domain::pool_location_names.at(pool_location).data()] =
                    entry_json;
        }

        package_json["poolEntries"] = pool_entries_json;

        const auto preferred_location =
            Core::Domain::select_preferred_pool_location(package.pool_entries);

        if (!preferred_location) {
            logd("Package {} has no pool entries, skipping preferred one "
                 "selection",
                 package.name);
            continue;
        }

        const auto preferred_candidate =
            package.pool_entries.at(*preferred_location);

        package_json["preferredCandidate"]["version"] =
            preferred_candidate.version;
        package_json["preferredCandidate"]["hasSignature"] =
            preferred_candidate.signature_path.has_value() ? "true" : "false";

        result.append(package_json);
    }

    co_return drogon::HttpResponse::newHttpJsonResponse(result);
}

drogon::Task<drogon::HttpResponsePtr>
    PackageController::snap(drogon::HttpRequestPtr req) {
    const auto sections_json = *req->getJsonObject();
    Json::Value result;

    if (sections_json.empty() || sections_json["source"].empty()
        || sections_json["target"].empty()) {
        result["error"] = "Invalid arguments";
        result["status"] = "error";

        auto response = drogon::HttpResponse::newHttpJsonResponse(result);
        response->setStatusCode(drogon::k400BadRequest);

        co_return response;
    }

    PackageSectionDTO source_branch {
        .branch = sections_json["source"]["branch"].asString(),
        .repository = sections_json["source"]["repository"].asString(),
        .architecture = sections_json["source"]["architecture"].asString()};

    BXT_JWT_CHECK_PERMISSIONS(
        (std::vector<std::string_view> {
            fmt::format("packages.snap.{}.{}.{}", source_branch.branch,
                        source_branch.repository, source_branch.architecture),
            fmt::format("sections.{}.{}.{}", source_branch.branch,
                        source_branch.repository, source_branch.architecture)}),
        req)

    PackageSectionDTO target_branch {
        .branch = sections_json["target"]["branch"].asString(),
        .repository = sections_json["target"]["repository"].asString(),
        .architecture = sections_json["target"]["architecture"].asString()};

    BXT_JWT_CHECK_PERMISSIONS(
        (std::vector<std::string_view> {
            fmt::format("packages.get.{}.{}.{}", target_branch.branch,
                        target_branch.repository, target_branch.architecture),
            fmt::format("sections.{}.{}.{}", target_branch.branch,
                        target_branch.repository, target_branch.architecture)}),
        req)

    const auto snap_ok =
        co_await m_package_service.snap(source_branch, target_branch);

    if (snap_ok.has_value()) {
        result["error"] = "Snap failed";
        result["status"] = "error";

        auto response = drogon::HttpResponse::newHttpJsonResponse(result);
        response->setStatusCode(drogon::k400BadRequest);

        co_return response;
    }

    result["status"] = "ok";
    co_return drogon::HttpResponse::newHttpJsonResponse(result);
}

} // namespace bxt::Presentation
