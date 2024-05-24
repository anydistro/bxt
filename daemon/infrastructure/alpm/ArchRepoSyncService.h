/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "ArchRepoOptions.h"
#include "core/application/services/SyncService.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "infrastructure/PackageFile.h"
#include "utilities/Error.h"
#include "utilities/eventbus/EventBusDispatcher.h"

#include <boost/uuid/uuid.hpp>
#include <coro/io_scheduler.hpp>
#include <coro/thread_pool.hpp>
#include <memory>
#include <vector>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

namespace bxt::Infrastructure {

class ArchRepoSyncService : public bxt::Core::Application::SyncService {
public:
    struct DownloadError : public bxt::Error {
        DownloadError(const std::string& package_filename,
                      const std::string& error_message)
            : package_filename(std::move(package_filename)) {
            message = error_message;
        }

        std::string package_filename;
    };
    BXT_DECLARE_RESULT(DownloadError);

    ArchRepoSyncService(Utilities::EventBusDispatcher& dispatcher,
                        PackageRepositoryBase& package_repository,
                        ArchRepoOptions& options)
        : m_dispatcher(dispatcher),
          m_package_repository(package_repository),
          m_options(options) {}

    coro::task<SyncService::Result<void>>
        sync(const PackageSectionDTO section) override;
    coro::task<SyncService::Result<void>> sync_all() override;

protected:
    coro::task<SyncService::Result<std::vector<Package>>>
        sync_section(const PackageSectionDTO section);

    coro::task<Result<std::vector<std::string>>>
        get_available_packages(const PackageSectionDTO section);
    coro::task<Result<PackageFile>>
        download_package(PackageSectionDTO section,
                         std::string package_filename,
                         boost::uuids::uuid id);

    coro::task<std::unique_ptr<httplib::SSLClient>>
        get_client(const std::string url);

    coro::task<std::optional<httplib::Result>> download_file(
        std::string url, std::string path, std::string filename = "");

private:
    Utilities::EventBusDispatcher& m_dispatcher;
    PackageRepositoryBase& m_package_repository;

    ArchRepoOptions m_options;
    coro::io_scheduler tp {{.pool = {.thread_count = 4}}};
};

} // namespace bxt::Infrastructure
