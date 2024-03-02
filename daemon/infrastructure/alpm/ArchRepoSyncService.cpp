/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "ArchRepoSyncService.h"

#include "boost/uuid/name_generator.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "core/application/events/IntegrationEventBase.h"
#include "core/application/events/SyncEvent.h"
#include "core/domain/entities/Package.h"
#include "core/domain/enums/PoolLocation.h"
#include "coro/sync_wait.hpp"
#include "coro/when_all.hpp"
#include "httplib.h"
#include "utilities/alpmdb/Desc.h"
#include "utilities/libarchive/Reader.h"
#include "utilities/log/Logging.h"

#include <coro/thread_pool.hpp>
#include <initializer_list>
#include <iterator>
#include <vector>

namespace bxt::Infrastructure {

coro::task<void> ArchRepoSyncService::sync(const PackageSectionDTO section) {
    using namespace Core::Application::Events;

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
        std::make_shared<SyncStarted>());

    auto all_packages = co_await sync_section(section);

    co_await m_package_repository.commit_async();

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
        std::make_shared<SyncFinished>(std::move(all_packages)));

    co_return;
}

coro::task<void> ArchRepoSyncService::sync_all() {
    using namespace Core::Application::Events;

    std::vector<coro::task<std::vector<Package>>> tasks;
    for (const auto& src : m_options.sources) {
        tasks.emplace_back(sync_section(src.first));
    }

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
        std::make_shared<SyncStarted>());

    auto package_lists = co_await coro::when_all(std::move(tasks));

    if (package_lists.empty()) { co_return; }

    auto& all_packages = package_lists[0].return_value();

    for (auto it = package_lists.begin() + 1; it != package_lists.end(); ++it) {
        auto& package_list = it->return_value();

        all_packages.insert(all_packages.end(),
                            std::make_move_iterator(package_list.begin()),
                            std::make_move_iterator(package_list.end()));
    }

    co_await m_package_repository.commit_async();

    co_await m_dispatcher.dispatch_single_async<IntegrationEventPtr>(
        std::make_shared<SyncFinished>(std::move(all_packages)));

    co_return;
}

coro::task<std::vector<Package>>
    ArchRepoSyncService::sync_section(const PackageSectionDTO section) {
    if (!m_options.sources.contains(section)) { co_return {}; }
    const auto remote_packages = co_await get_available_packages(section);

    std::vector<coro::task<void>> tasks;

    const auto uuid = boost::uuids::random_generator()();
    std::vector<Package> packages;
    packages.reserve(remote_packages.size());

    auto task = [this, section, uuid,
                 &packages](const auto pkgname) -> coro::task<void> {
        const auto package_file =
            co_await download_package(section, pkgname, uuid);

        auto package_result = Package::from_file_path(
            SectionDTOMapper::to_entity(package_file.section()),
            Core::Domain::PoolLocation::Sync, package_file.file_path());

        if (package_result.has_value()) {
            packages.emplace_back(std::move(*package_result));
        }
    };

    for (const auto& pkgname : remote_packages) {
        tasks.emplace_back(task(pkgname));
    }

    co_await coro::when_all(std::move(tasks));

    co_await m_package_repository.add_async(packages);

    co_return packages;
}

coro::task<std::vector<std::string>>
    ArchRepoSyncService::get_available_packages(
        const PackageSectionDTO section) {
    std::vector<std::string> result;

    const auto client =
        co_await get_client(m_options.sources[section].repo_url);

    const auto repository_name =
        m_options.sources[section].repo_name.value_or(section.repository);

    const auto db_path_format =
        fmt::format("{}/{{repository}}.db",
                    m_options.sources[section].repo_structure_template);

    const auto path = fmt::format(
        fmt::runtime(db_path_format), fmt::arg("branch", section.branch),
        fmt::arg("repository", repository_name),
        fmt::arg("architecture", section.architecture));

    auto response = client->Get(path, httplib::Headers());

    if (response.error() != httplib::Error::Success) { co_return {}; }

    if (response->status != 200) { co_return result; }

    Archive::Reader reader;

    archive_read_support_filter_all(reader);
    archive_read_support_format_all(reader);

    reader.open_memory(reinterpret_cast<uint8_t*>(response->body.data()),
                       response->body.size());

    for (auto& [header, entry] : reader) {
        std::string pname = archive_entry_pathname(*header);

        if (!pname.ends_with("/desc")) { continue; }

        auto contents = entry.read_all();

        if (!contents.has_value()) { continue; }

        Utilities::AlpmDb::Desc desc(reinterpret_cast<char*>(contents->data()));

        const auto filename = desc.get("FILENAME");
        if (!filename.has_value()) { continue; }

        const auto name = desc.get("NAME");
        if (!name.has_value()) { continue; }

        auto version_field = desc.get("VERSION");

        if (!version_field.has_value()) { co_return {}; }

        const auto version =
            Core::Domain::PackageVersion::from_string(*version_field);

        if (!version.has_value()) { continue; }

        const auto existing_package =
            co_await m_package_repository.find_by_section_async(
                SectionDTOMapper::to_entity(section), *name);

        if (existing_package.has_value()) {
            if (existing_package->version() < *version) {
                result.emplace_back(*filename);
            }
        } else {
            result.emplace_back(*filename);
        }
    }
    co_return result;
}

coro::task<PackageFile>
    ArchRepoSyncService::download_package(PackageSectionDTO section,
                                          std::string package_filename,
                                          boost::uuids::uuid id) {
    const auto client =
        co_await get_client(m_options.sources[section].repo_url);

    auto repository_name =
        m_options.sources[section].repo_name.value_or(section.repository);

    auto path_format = fmt::format(
        "{}/{{pkgfname}}", m_options.sources[section].repo_structure_template);

    auto path = fmt::format(fmt::runtime(path_format),
                            fmt::arg("branch", section.branch),
                            fmt::arg("repository", repository_name),
                            fmt::arg("architecture", section.architecture),
                            fmt::arg("pkgfname", package_filename));

    auto filepath = m_options.sources[section].download_path
                    / std::string(boost::uuids::to_string(id));

    std::filesystem::create_directories(filepath);

    auto full_filename =
        fmt::format("{}/{}", filepath.string(), package_filename);

    if (std::filesystem::exists(full_filename)) {
        co_return PackageFile(section, full_filename);
    }

    logd("Sync: Downloading {} as {}...", package_filename, full_filename);

    std::ofstream stream(full_filename);

    auto response =
        client->Get(path, [&](const char* data, size_t data_length) {
            stream.write(data, data_length);
            return true;
        });

    co_return PackageFile(section, full_filename);
}

coro::task<std::unique_ptr<httplib::SSLClient>>
    ArchRepoSyncService::get_client(const std::string url) {
    co_await tp.schedule();

    auto client_ptr = std::make_unique<httplib::SSLClient>(url);

    client_ptr->set_follow_location(true);
    client_ptr->enable_server_certificate_verification(true);

    co_return client_ptr;
}

} // namespace bxt::Infrastructure
