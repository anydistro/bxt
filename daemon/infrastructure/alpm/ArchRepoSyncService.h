/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "ArchRepoOptions.h"
#include "boost/uuid/uuid.hpp"
#include "core/application/services/SyncService.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "dexode/EventBus.hpp"
#include "infrastructure/PackageFile.h"
#include "utilities/eventbus/EventBusDispatcher.h"

#include <coro/thread_pool.hpp>
#include <memory>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

namespace bxt::Infrastructure {

class ArchRepoSyncService : public bxt::Core::Application::SyncService {
public:
    ArchRepoSyncService(Utilities::EventBusDispatcher& dispatcher,
                        PackageRepositoryBase& package_repository,
                        ArchRepoOptions& options)
        : m_dispatcher(dispatcher),
          m_package_repository(package_repository),
          m_options(options) {}

    virtual coro::task<void> sync(const PackageSectionDTO section) override;
    virtual coro::task<void> sync_all() override;

protected:
    coro::task<std::vector<Package>>
        sync_section(const PackageSectionDTO section);

    coro::task<std::vector<std::string>>
        get_available_packages(const PackageSectionDTO section);
    coro::task<PackageFile> download_package(PackageSectionDTO section,
                                             std::string package_filename,
                                             boost::uuids::uuid id);

    coro::task<std::unique_ptr<httplib::SSLClient>>
        get_client(const std::string url);

private:
    Utilities::EventBusDispatcher& m_dispatcher;
    PackageRepositoryBase& m_package_repository;

    ArchRepoOptions m_options;
    coro::thread_pool tp {coro::thread_pool::options {.thread_count = 4}};
};

} // namespace bxt::Infrastructure
