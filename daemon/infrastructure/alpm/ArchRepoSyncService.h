/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "ArchRepoOptions.h"
#include "core/application/RequestContext.h"
#include "core/application/services/SyncService.h"
#include "core/domain/entities/Package.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
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
        DownloadError(std::string const& package_filename, std::string const& error_message)
            : package_filename(std::move(package_filename)) {
            message = error_message;
        }

        std::string package_filename;
    };
    BXT_DECLARE_RESULT(DownloadError);

    struct PackageInfo {
        std::string name;
        std::string filename;
        Core::Domain::PackageVersion version;
        std::string hash;
        std::optional<std::string> signature;
    };

    ArchRepoSyncService(Utilities::EventBusDispatcher& dispatcher,
                        PackageRepositoryBase& package_repository,
                        ArchRepoOptions& options,
                        UnitOfWorkBaseFactory& uow_factory)
        : m_dispatcher(dispatcher)
        , m_package_repository(package_repository)
        , m_options(options)
        , m_uow_factory(uow_factory) {
    }

    coro::task<SyncService::Result<void>> sync(PackageSectionDTO const section,
                                               RequestContext const context) override;
    coro::task<SyncService::Result<void>> sync_all(RequestContext const context) override;

protected:
    coro::task<SyncService::Result<std::vector<Package>>>
        sync_section(PackageSectionDTO const section);

    coro::task<Result<std::vector<PackageInfo>>>
        get_available_packages(PackageSectionDTO const section);
    coro::task<Result<Package>>
        download_package(PackageSectionDTO section,
                         std::string package_filename,
                         std::string sha256_hash,
                         std::optional<std::string> signature = std::nullopt);

    coro::task<std::unique_ptr<httplib::SSLClient>> get_client(std::string const url);

    coro::task<std::optional<httplib::Result>>
        download_file(std::string url, std::string path, std::string filename = "");

    bool is_excluded(PackageSectionDTO const& section, std::string const& package_name) const;

private:
    Utilities::EventBusDispatcher& m_dispatcher;
    PackageRepositoryBase& m_package_repository;
    UnitOfWorkBaseFactory& m_uow_factory;

    ArchRepoOptions m_options;
    coro::io_scheduler tp {{.pool = {.thread_count = 1}}};
};

} // namespace bxt::Infrastructure
