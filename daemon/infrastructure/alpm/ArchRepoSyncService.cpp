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
#include "utilities/Error.h"
#include "utilities/alpmdb/Desc.h"
#include "utilities/libarchive/Reader.h"
#include "utilities/log/Logging.h"

#include <boost/uuid/uuid_io.hpp>
#include <coro/sync_wait.hpp>
#include <coro/thread_pool.hpp>
#include <coro/when_all.hpp>
#include <expected>
#include <httplib.h>
#include <ios>
#include <iterator>
#include <memory>
#include <nonstd/scope.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <system_error>
#include <vector>

namespace bxt::Infrastructure {

coro::task<SyncService::Result<void>>
    ArchRepoSyncService::sync(const PackageSectionDTO section) {
    using namespace Core::Application::Events;

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
        std::make_shared<SyncStarted>());

    auto all_packages = co_await sync_section(section);
    if (!all_packages.has_value()) {
        co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
            std::make_shared<SyncFinished>(std::move(*all_packages)));

        co_return std::unexpected(all_packages.error());
    }
    co_await m_package_repository.commit_async();

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
        std::make_shared<SyncFinished>(std::move(*all_packages)));

    co_return {};
}

coro::task<SyncService::Result<void>> ArchRepoSyncService::sync_all() {
    using namespace Core::Application::Events;

    auto tasks = m_options.sources
                 | std::views::transform([this](const auto& src) {
                       return sync_section(src.first);
                   })
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
        co_return std::unexpected(all_packages.error());
    }

    for (auto it = package_lists.begin() + 1; it != package_lists.end(); ++it) {
        auto& package_list = it->return_value();

        if (!package_list.has_value()) {
            co_return std::unexpected(package_list.error());
        }

        all_packages->insert(all_packages->end(),
                             std::make_move_iterator(package_list->begin()),
                             std::make_move_iterator(package_list->end()));
    }

    auto commit_ok = co_await m_package_repository.commit_async();

    if (!commit_ok.has_value()) {
        co_return bxt::make_error_with_source<SyncError>(
            std::move(commit_ok.error()), SyncError::RepositoryError);
    }

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
        std::make_shared<SyncFinished>(std::move(*all_packages)));
    guard.release();
    co_return {};
}

coro::task<SyncService::Result<std::vector<Package>>>
    ArchRepoSyncService::sync_section(const PackageSectionDTO section) {
    if (!m_options.sources.contains(section)) { co_return {}; }
    auto remote_packages = co_await get_available_packages(section);

    if (!remote_packages.has_value()) {
        co_return bxt::make_error_with_source<SyncError>(
            std::move(remote_packages.error()), SyncError::NetworkError);
    }

    const auto uuid = boost::uuids::random_generator()();

    auto task = [this, section, uuid](const auto pkgname)
        -> coro::task<ArchRepoSyncService::Result<Package>> {
        auto package_file = co_await download_package(section, pkgname, uuid);

        if (!package_file) { co_return std::unexpected(package_file.error()); }

        auto package_result = Package::from_file_path(
            SectionDTOMapper::to_entity(package_file->section()),
            Core::Domain::PoolLocation::Sync, package_file->file_path());

        if (!package_result.has_value()) {
            co_return bxt::make_error_with_source<DownloadError>(
                std::move(package_result.error()), package_file->file_path(),
                "Parse error");
        }

        co_return *package_result;
    };

    auto tasks = *remote_packages | std::views::transform(task)
                 | std::ranges::to<std::vector>();

    const auto package_results = co_await coro::when_all(std::move(tasks));
    std::vector<Package> packages;
    packages.reserve(package_results.size());
    for (const auto& package_result : package_results) {
        auto& package = package_result.return_value();

        if (!package.has_value()) {
            loge("Download of {} has failed. The reason is \"{}\". Aborting "
                 "sync.",
                 package.error().package_filename, package.error().what());

            co_return bxt::make_error_with_source<SyncError>(
                std::move(package.error()), SyncError::RepositoryError);
        }

        co_await m_package_repository.add_async(*package);
        packages.emplace_back(std::move(*package));
    }

    co_return packages;
}

struct ParseResult {
    std::string name;
    std::string filename;
    Core::Domain::PackageVersion version;
};

std::optional<ParseResult> parse_descfile(auto& entry) {
    auto contents = entry.read_all();

    if (!contents.has_value()) { return {}; }

    Utilities::AlpmDb::Desc desc(reinterpret_cast<char*>(contents->data()));

    const auto filename = desc.get("FILENAME");
    if (!filename.has_value()) { return {}; }

    const auto name = desc.get("NAME");
    if (!name.has_value()) { return {}; }

    auto version_field = desc.get("VERSION");

    if (!version_field.has_value()) { return {}; }

    const auto version =
        Core::Domain::PackageVersion::from_string(*version_field);

    if (!version.has_value()) { return {}; }

    return ParseResult {
        .name = *name, .filename = *filename, .version = *version};
}
coro::task<ArchRepoSyncService::Result<std::vector<std::string>>>
    ArchRepoSyncService::get_available_packages(
        const PackageSectionDTO section) {
    std::vector<std::string> result;

    const auto repository_name =
        m_options.sources[section].repo_name.value_or(section.repository);

    const auto db_path_format =
        fmt::format("{}/{{repository}}.db",
                    m_options.sources[section].repo_structure_template);

    const auto path = fmt::format(
        fmt::runtime(db_path_format), fmt::arg("branch", section.branch),
        fmt::arg("repository", repository_name),
        fmt::arg("architecture", section.architecture));

    auto download_result =
        co_await download_file(m_options.sources[section].repo_url, path);

    if (!download_result.has_value()) {
        co_return bxt::make_error<DownloadError>(path,
                                                 "Can't download the database");
    }
    if (!(*download_result)) {
        co_return bxt::make_error<DownloadError>(
            path, httplib::to_string(download_result->error()));
    }

    auto response = download_result->value();

    if (response.status != 200) {
        co_return bxt::make_error<DownloadError>(path,
                                                 "The response is non-200");
    }

    Archive::Reader reader;

    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);

    auto open_ok = reader.open_memory(
        reinterpret_cast<uint8_t*>(response.body.data()), response.body.size());

    if (!open_ok.has_value()) {
        co_return bxt::make_error_with_source<DownloadError>(
            std::move(open_ok.error()), path, "The archive cannot be opened");
    }

    for (auto& [header, entry] : reader) {
        std::string pname = archive_entry_pathname(*header);

        if (!pname.ends_with("/desc")) { continue; }

        auto parsed_package_info = parse_descfile(entry);

        if (!parsed_package_info.has_value()) {
            co_return bxt::make_error<DownloadError>(
                "Unknown", fmt::format("Cannot parse descfile {}", pname));
        }

        const auto& [name, filename, version] = *parsed_package_info;

        if (is_excluded(section, name)) {
            logi("Package {} is excluded. Skipping.", name);
            continue;
        }

        const auto existing_package =
            co_await m_package_repository.find_by_section_async(
                SectionDTOMapper::to_entity(section), name);
        if (existing_package.has_value()) {
            if (existing_package->version() < version) {
                result.emplace_back(filename);
            }
        } else {
            result.emplace_back(filename);
        }
    }
    co_return result;
}

coro::task<ArchRepoSyncService::Result<PackageFile>>
    ArchRepoSyncService::download_package(PackageSectionDTO section,
                                          std::string package_filename,
                                          boost::uuids::uuid id) {
    const auto repository_name =
        m_options.sources[section].repo_name.value_or(section.repository);

    const auto path_format = fmt::format(
        "{}/{{pkgfname}}", m_options.sources[section].repo_structure_template);

    const auto path = fmt::format(
        fmt::runtime(path_format), fmt::arg("branch", section.branch),
        fmt::arg("repository", repository_name),
        fmt::arg("architecture", section.architecture),
        fmt::arg("pkgfname", package_filename));

    const auto filepath = m_options.sources[section].download_path
                          / std::string(boost::uuids::to_string(id));

    std::error_code ec;
    if (std::filesystem::create_directories(filepath, ec); ec) {
        co_return bxt::make_error<DownloadError>(
            package_filename, "Cannot create directory: " + ec.message());
    }

    const auto full_filename =
        fmt::format("{}/{}", filepath.string(), package_filename);

    if (std::filesystem::exists(full_filename)) {
        co_return PackageFile(section, full_filename);
    }

    auto response = co_await download_file(m_options.sources[section].repo_url,
                                           path, full_filename);

    if (!response.has_value()) {
        co_return bxt::make_error<DownloadError>(package_filename,
                                                 "Can't download the package");
    }
    if (!(*response)) {
        co_return bxt::make_error<DownloadError>(
            package_filename, httplib::to_string(response->error()));
    }

    response = co_await download_file(m_options.sources[section].repo_url,
                                      path + ".sig", full_filename + ".sig");

    if (!response.has_value()) {
        co_return bxt::make_error<DownloadError>(
            package_filename + ".sig", "Can't download the signature");
    }
    if (!(*response)) {
        co_return bxt::make_error<DownloadError>(
            package_filename + ".sig", httplib::to_string(response->error()));
    }

    co_return PackageFile(section, full_filename);
}
coro::task<std::optional<httplib::Result>> ArchRepoSyncService::download_file(
    std::string url, std::string path, std::string filename) {
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

            response = client->Get(path, [&](const char* data,
                                             size_t data_length) {
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

bool ArchRepoSyncService::is_excluded(const PackageSectionDTO& section,
                                      const std::string& package_name) const {
    const auto& exclude_list = m_options.sources.at(section).exclude_list;
    for (const auto& pattern : exclude_list) {
        if (std::regex_match(package_name, std::regex(pattern))) {
            return true;
        }
    }

    return false;
}

coro::task<std::unique_ptr<httplib::SSLClient>>
    ArchRepoSyncService::get_client(const std::string url) {
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
