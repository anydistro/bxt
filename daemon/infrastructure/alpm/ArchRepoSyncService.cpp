/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "ArchRepoSyncService.h"

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/events/IntegrationEventBase.h"
#include "core/application/events/SyncEvent.h"
#include "core/domain/entities/Package.h"
#include "core/domain/enums/PoolLocation.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "utilities/alpmdb/Desc.h"
#include "utilities/base64.h"
#include "utilities/Error.h"
#include "utilities/hash_from_file.h"
#include "utilities/libarchive/Reader.h"
#include "utilities/log/Logging.h"
#include "utilities/to_string.h"

#include <coro/sync_wait.hpp>
#include <coro/thread_pool.hpp>
#include <coro/when_all.hpp>
#include <expected>
#include <httplib.h>
#include <ios>
#include <iterator>
#include <memory>
#include <nonstd/scope.hpp>
#include <openssl/sha.h>
#include <optional>
#include <ranges>
#include <string>
#include <system_error>
#include <vector>

namespace bxt::Infrastructure {

coro::task<SyncService::Result<void>> ArchRepoSyncService::sync(PackageSectionDTO const section,
                                                                RequestContext const context) {
    using namespace Core::Application::Events;

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
        std::make_shared<SyncStarted>());

    auto uow = co_await m_uow_factory(true);

    auto all_packages = co_await sync_section(section);
    if (!all_packages.has_value()) {
        co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
            std::make_shared<SyncFinished>(std::move(*all_packages),
                                           std::vector<bxt::Core::Domain::Package::TId> {},
                                           context.user_name));

        co_return std::unexpected(all_packages.error());
    }
    co_await uow->commit_async();

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(std::make_shared<SyncFinished>(
        std::move(*all_packages), std::vector<bxt::Core::Domain::Package::TId> {},
        context.user_name));

    co_return {};
}

coro::task<SyncService::Result<void>> ArchRepoSyncService::sync_all(RequestContext const context) {
    using namespace Core::Application::Events;

    auto tasks =
        m_options.sources
        | std::views::transform([this](auto const& src) { return sync_section(src.first); })
        | std::ranges::to<std::vector>();

    auto guard = nonstd::make_scope_exit([this]() {
        coro::sync_wait(m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
            std::make_shared<SyncFinished>()));
    });

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
        std::make_shared<SyncStarted>());

    auto package_lists = co_await coro::when_all(std::move(tasks));

    if (package_lists.empty()) {
        logi("No packages to sync");
        co_return {};
    }

    auto& all_packages = package_lists[0].return_value();
    if (!all_packages.has_value()) {
        loge("Failed to sync packages: {}", all_packages.error().what());
        co_return std::unexpected(all_packages.error());
    }

    for (auto it = package_lists.begin() + 1; it != package_lists.end(); ++it) {
        auto& package_list = it->return_value();

        if (!package_list.has_value()) {
            loge("Failed to sync packages: {}", all_packages.error().what());

            co_return std::unexpected(package_list.error());
        }

        all_packages->insert(all_packages->end(), std::make_move_iterator(package_list->begin()),
                             std::make_move_iterator(package_list->end()));
    }

    logi("Downloaded {} packages, saving to database...", all_packages->size());

    auto uow = co_await m_uow_factory(true);

    auto saved = co_await m_package_repository.save_async(*all_packages, uow);
    if (!saved.has_value()) {
        loge("Failed to save packages: {}", saved.error().what());
        co_return bxt::make_error_with_source<SyncError>(std::move(saved.error()),
                                                         SyncError::RepositoryError);
    }

    auto commit_ok = co_await uow->commit_async();

    if (!commit_ok.has_value()) {
        loge("Failed to save packages: {}", saved.error().what());
        co_return bxt::make_error_with_source<SyncError>(std::move(commit_ok.error()),
                                                         SyncError::RepositoryError);
    }

    logi("Saved {} packages to database", all_packages->size());

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(std::make_shared<SyncFinished>(
        std::move(*all_packages), std::vector<bxt::Core::Domain::Package::TId> {},
        context.user_name));
    guard.release();
    co_return {};
}

coro::task<SyncService::Result<std::vector<Package>>>
    ArchRepoSyncService::sync_section(PackageSectionDTO const section) {
    if (!m_options.sources.contains(section)) {
        co_return {};
    }
    constexpr int max_retries = 5;
    std::vector<Package> packages;

    for (int attempt = 0; attempt < max_retries; ++attempt) {
        auto remote_packages = co_await get_available_packages(section);

        if (!remote_packages.has_value()) {
            if (attempt == max_retries - 1) {
                co_return bxt::make_error_with_source<SyncError>(std::move(remote_packages.error()),
                                                                 SyncError::NetworkError);
            }
            continue;
        }

        auto task = [this, section](PackageInfo const pkginfo) -> coro::task<Result<Package>> {
            co_return co_await download_package(section, pkginfo.filename, pkginfo.hash,
                                                pkginfo.signature);
        };

        auto tasks =
            *remote_packages | std::views::transform(task) | std::ranges::to<std::vector>();

        auto const package_results = co_await coro::when_all(std::move(tasks));
        bool all_downloads_successful = true;
        packages.clear();
        packages.reserve(package_results.size());

        for (auto const& package_result : package_results) {
            auto& package = package_result.return_value();

            if (!package.has_value()) {
                loge("Download of {} has failed. The reason is \"{}\". "
                     "Retrying...",
                     package.error().package_filename, package.error().what());
                all_downloads_successful = false;
                break;
            }

            packages.emplace_back(std::move(*package));
        }

        if (all_downloads_successful) {
            break;
        }

        if (attempt == max_retries - 1) {
            loge("Max retries reached. Some package downloads failed.");
            co_return bxt::make_error<SyncError>(SyncError::NetworkError);
        }

        logi("Retrying to sync packages, attempt {}", attempt + 1);
    }

    co_return packages;
}

std::optional<ArchRepoSyncService::PackageInfo> parse_descfile(auto& entry) {
    auto contents = entry.read_all();

    if (!contents.has_value()) {
        return {};
    }

    Utilities::AlpmDb::Desc desc(reinterpret_cast<char*>(contents->data()));

    auto const filename = desc.get("FILENAME");
    if (!filename.has_value()) {
        return {};
    }

    auto const name = desc.get("NAME");
    if (!name.has_value()) {
        return {};
    }

    auto version_field = desc.get("VERSION");

    if (!version_field.has_value()) {
        return {};
    }

    auto const version = Core::Domain::PackageVersion::from_string(*version_field);

    if (!version.has_value()) {
        return {};
    }

    auto const hash = desc.get("SHA256SUM");

    if (!hash.has_value()) {
        return {};
    }

    auto signature = desc.get("PGPSIG");

    if (signature.has_value()) {
        signature = bxt::Utilities::b64_decode(*signature);
    }

    return ArchRepoSyncService::PackageInfo {.name = *name,
                                             .filename = *filename,
                                             .version = *version,
                                             .hash = *hash,
                                             .signature = signature};
}
coro::task<ArchRepoSyncService::Result<std::vector<ArchRepoSyncService::PackageInfo>>>
    ArchRepoSyncService::get_available_packages(PackageSectionDTO const section) {
    std::vector<PackageInfo> result;

    auto const repository_name = m_options.sources[section].repo_name.value_or(section.repository);

    auto const db_path_format =
        fmt::format("{}/{{repository}}.db", m_options.sources[section].repo_structure_template);

    auto const path = fmt::format(fmt::runtime(db_path_format), fmt::arg("branch", section.branch),
                                  fmt::arg("repository", repository_name),
                                  fmt::arg("architecture", section.architecture));

    auto download_result = co_await download_file(m_options.sources[section].repo_url, path);

    if (!download_result.has_value()) {
        co_return bxt::make_error<DownloadError>(path, "Can't download the database");
    }
    if (!(*download_result)) {
        co_return bxt::make_error<DownloadError>(path,
                                                 httplib::to_string(download_result->error()));
    }

    auto response = download_result->value();

    if (response.status != 200) {
        co_return bxt::make_error<DownloadError>(path, "The response is non-200");
    }

    Archive::Reader reader;

    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);

    auto open_ok =
        reader.open_memory(reinterpret_cast<uint8_t*>(response.body.data()), response.body.size());

    if (!open_ok.has_value()) {
        co_return bxt::make_error_with_source<DownloadError>(std::move(open_ok.error()), path,
                                                             "The archive cannot be opened");
    }
    auto uow = co_await m_uow_factory();
    for (auto& [header, entry] : reader) {
        std::string pname = archive_entry_pathname(*header);

        if (!pname.ends_with("/desc")) {
            continue;
        }

        auto parsed_package_info = parse_descfile(entry);

        if (!parsed_package_info.has_value()) {
            co_return bxt::make_error<DownloadError>(
                "Unknown", fmt::format("Cannot parse descfile {}", pname));
        }

        auto const& [name, filename, version, hash, signature] = *parsed_package_info;

        if (is_excluded(section, name)) {
            logi("Package {} is excluded. Skipping.", name);
            continue;
        }

        auto const existing_package = co_await m_package_repository.find_by_section_async(
            SectionDTOMapper::to_entity(section), name, uow);
        if (existing_package.has_value()) {
            if (existing_package->version() < version) {
                result.emplace_back(std::move(*parsed_package_info));
            }
        } else {
            result.emplace_back(std::move(*parsed_package_info));
        }
    }
    co_return result;
}

coro::task<ArchRepoSyncService::Result<Package>>
    ArchRepoSyncService::download_package(PackageSectionDTO section,
                                          std::string package_filename,
                                          std::string sha256_hash,
                                          std::optional<std::string> signature) {
    auto const repository_name = m_options.sources[section].repo_name.value_or(section.repository);

    auto const path_format =
        fmt::format("{}/{{pkgfname}}", m_options.sources[section].repo_structure_template);

    auto const path = fmt::format(fmt::runtime(path_format), fmt::arg("branch", section.branch),
                                  fmt::arg("repository", repository_name),
                                  fmt::arg("architecture", section.architecture),
                                  fmt::arg("pkgfname", package_filename));

    auto const filepath = m_options.download_path / bxt::to_string(section);

    std::error_code ec;
    if (std::filesystem::create_directories(filepath, ec); ec) {
        co_return bxt::make_error<DownloadError>(package_filename,
                                                 "Cannot create directory: " + ec.message());
    }

    auto const full_filename = fmt::format("{}/{}", filepath.string(), package_filename);

    if (std::filesystem::exists(full_filename)) {
        logi("Found package file in local cache: {}, checking the hash... ", full_filename);

        if (bxt::hash_from_file<SHA256, SHA256_DIGEST_LENGTH>(full_filename) == sha256_hash) {
            logi("Hash is ok. Using local cache package file: {}", full_filename);
        } else {
            logw("Hash is wrong. Invalid package file: {}, removing it", full_filename);
            std::filesystem::remove(full_filename);
        }
    }
    if (!std::filesystem::exists(full_filename)) {
        auto response =
            co_await download_file(m_options.sources[section].repo_url, path, full_filename);

        if (!response.has_value()) {
            co_return bxt::make_error<DownloadError>(package_filename,
                                                     "Can't download the package");
        }
        if (!(*response)) {
            co_return bxt::make_error<DownloadError>(package_filename,
                                                     httplib::to_string(response->error()));
        }
    }
    if (signature == std::nullopt) {
        logi("Signature was not found in downloaded database."
             "Trying to download it from the repository...");
        auto response = co_await download_file(m_options.sources[section].repo_url, path + ".sig",
                                               full_filename + ".sig");

        if (!response.has_value()) {
            co_return bxt::make_error<DownloadError>(package_filename + ".sig",
                                                     "Can't download the signature");
        }
        if (!(*response)) {
            co_return bxt::make_error<DownloadError>(package_filename + ".sig",
                                                     httplib::to_string(response->error()));
        }
    } else {
        logi("Signature was found in downloaded database, writing it to file.");
        std::ofstream sig_file(full_filename + ".sig", std::ios::binary);
        if (!sig_file) {
            co_return bxt::make_error<DownloadError>(package_filename + ".sig",
                                                     "Can't create signature file");
        }
        sig_file.write(signature->data(), signature->size());
        if (!sig_file) {
            co_return bxt::make_error<DownloadError>(package_filename + ".sig",
                                                     "Can't write to signature file");
        }
    }

    auto result = Package::from_file_path(SectionDTOMapper::to_entity(section),
                                          Core::Domain::PoolLocation::Sync, full_filename);

    if (result.has_value()) {
        co_return result.value();
    } else {
        co_return bxt::make_error_with_source<DownloadError>(
            std::move(result.error()), package_filename, "Can't parse the package");
    }
}
coro::task<std::optional<httplib::Result>>
    ArchRepoSyncService::download_file(std::string url, std::string path, std::string filename) {
    using namespace std::chrono_literals;
    constexpr int retry_max = 5;
    constexpr auto delay = 50ms;

    int current_retry = 0;

    std::optional<httplib::Result> response;

    while (current_retry < retry_max) {
        auto client = co_await get_client(url);
        if (!client) {
            loge("Failed to get client for URL: {}", url);
            co_return {};
        }

        if (!filename.empty()) {
            std::ofstream stream(filename, std::ios::binary);
            if (!stream.is_open()) {
                loge("Failed to open file: {}", filename);
                co_return {};
            }

            response = client->Get(path, [&](char const* data, size_t data_length) {
                stream.write(data, static_cast<std::streamsize>(data_length));
                return stream.good();
            });

            stream.close();

            if (!stream) {
                loge("Failed to write to file: {}", filename);
                co_return {};
            }
        } else {
            response = client->Get(path, httplib::Headers());
        }

        if (response && response->error() == httplib::Error::Success
            && response->value().status == 200) {
            logi("Successfully downloaded file: {}", path);
            co_return response;
        }

        logw("Failed to download file: {}, retrying...", path);
        co_await tp.yield_for(delay);
        ++current_retry;
    }

    loge("Failed to download file: {} after {} retries", path, retry_max);
    co_return response;
}

bool ArchRepoSyncService::is_excluded(PackageSectionDTO const& section,
                                      std::string const& package_name) const {
    auto const& exclude_list = m_options.sources.at(section).exclude_list;
    for (auto const& pattern : exclude_list) {
        if (std::regex_match(package_name, std::regex(pattern))) {
            return true;
        }
    }

    return false;
}

coro::task<std::unique_ptr<httplib::SSLClient>>
    ArchRepoSyncService::get_client(std::string const url) {
    using namespace std::chrono_literals;
    constexpr static auto timeout = 5s;

    co_await tp.schedule();

    auto client_ptr = std::make_unique<httplib::SSLClient>(url);

    client_ptr->set_follow_location(true);
    client_ptr->enable_server_certificate_verification(true);
    client_ptr->set_connection_timeout(timeout);

    co_return client_ptr;
}

} // namespace bxt::Infrastructure
