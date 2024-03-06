/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/services/AuthService.h"
#include "core/application/services/CompareService.h"
#include "core/application/services/PackageLogEntryService.h"
#include "core/application/services/PackageService.h"
#include "core/application/services/PermissionService.h"
#include "core/application/services/SectionService.h"
#include "core/application/services/UserService.h"
#include "core/domain/entities/PackageLogEntry.h"
#include "coro/io_scheduler.hpp"
#include "infrastructure/DeploymentService.h"
#include "infrastructure/DispatchingUnitOfWork.h"
#include "infrastructure/PackageService.h"
#include "infrastructure/alpm/ArchRepoOptions.h"
#include "infrastructure/alpm/ArchRepoSyncService.h"
#include "infrastructure/ws/WSController.h"
#include "kangaru/autowire.hpp"
#include "kangaru/detail/single.hpp"
#include "kangaru/operator_service.hpp"
#include "kangaru/service.hpp"
#include "persistence/box/BoxRepository.h"
#include "persistence/box/export/AlpmDBExporter.h"
#include "persistence/box/export/ExporterBase.h"
#include "persistence/box/pool/Pool.h"
#include "persistence/box/pool/PoolBase.h"
#include "persistence/box/pool/PoolOptions.h"
#include "persistence/box/store/LMDBPackageStore.h"
#include "persistence/box/store/PackageStoreBase.h"
#include "persistence/box/writeback/WritebackScheduler.h"
#include "persistence/config/SectionRepository.h"
#include "persistence/lmdb/PackageLogEntryRepository.h"
#include "persistence/lmdb/UserRepository.h"
#include "presentation/cli-controllers/DeploymentController.h"
#include "presentation/web-controllers/AuthController.h"
#include "presentation/web-controllers/CompareController.h"
#include "presentation/web-controllers/LogController.h"
#include "presentation/web-controllers/PackageController.h"
#include "presentation/web-controllers/SectionController.h"
#include "presentation/web-controllers/UserController.h"
#include "presentation/web-filters/JwtFilter.h"
#include "utilities/lmdb/Environment.h"
#include "utilities/repo-schema/Parser.h"

#include <infrastructure/EventLogger.h>
#include <kangaru/debug.hpp>
#include <kangaru/kangaru.hpp>

template<auto m> using method = kgr::method<decltype(m), m>;

namespace bxt::di {

namespace Utilities {

    struct EventBus : kgr::shared_service<dexode::EventBus> {};

    struct EventBusDispatcher
        : kgr::single_service<bxt::Utilities::EventBusDispatcher,
                              kgr::dependency<di::Utilities::EventBus>> {};

    struct IOScheduler : kgr::shared_service<coro::io_scheduler> {};

    namespace LMDB {

        struct Environment
            : kgr::shared_service<bxt::Utilities::LMDB::Environment,
                                  kgr::dependency<IOScheduler>> {};

    } // namespace LMDB

    namespace RepoSchema {

        struct Parser
            : kgr::autowire_single_service<bxt::Utilities::RepoSchema::Parser> {
        };

    } // namespace RepoSchema

} // namespace Utilities

namespace Core {

    namespace Domain {

        struct UserRepository
            : kgr::abstract_service<
                  bxt::Core::Domain::ReadWriteRepositoryBase<User>> {};

        struct PackageRepositoryBase
            : kgr::abstract_service<bxt::Core::Domain::PackageRepositoryBase> {
        };

        struct ReadOnlySectionRepository
            : kgr::abstract_service<
                  bxt::Core::Domain::ReadOnlyRepositoryBase<Section>> {};

        struct PackageLogEntryRepository
            : kgr::abstract_service<
                  bxt::Core::Domain::ReadWriteRepositoryBase<PackageLogEntry>> {
        };

    } // namespace Domain

    namespace Application {

        struct AuthService
            : kgr::single_service<
                  bxt::Core::Application::AuthService,
                  kgr::dependency<di::Core::Domain::UserRepository>> {};

        struct DeploymentService
            : kgr::abstract_service<bxt::Core::Application::DeploymentService> {
        };

        struct PackageService
            : kgr::abstract_service<bxt::Core::Application::PackageService> {};

        struct SyncService
            : kgr::abstract_service<bxt::Core::Application::SyncService> {};

        struct UserService
            : kgr::single_service<
                  bxt::Core::Application::UserService,
                  kgr::dependency<di::Core::Domain::UserRepository>> {};

        struct PermissionService
            : kgr::single_service<
                  bxt::Core::Application::PermissionService,
                  kgr::dependency<di::Core::Domain::UserRepository>> {};

        struct PackageLogEntryService
            : kgr::single_service<
                  bxt::Core::Application::PackageLogEntryService,
                  kgr::dependency<di::Utilities::EventBus,
                                  di::Core::Domain::PackageLogEntryRepository>>,
              kgr::autocall<kgr::invoke<method<
                  &bxt::Core::Application::PackageLogEntryService::init>>> {};

        struct SectionService
            : kgr::single_service<
                  bxt::Core::Application::SectionService,
                  kgr::dependency<
                      di::Core::Domain::ReadOnlySectionRepository>> {};

        struct CompareService
            : kgr::single_service<
                  bxt::Core::Application::CompareService,
                  kgr::dependency<di::Core::Application::PackageService>> {};

    } // namespace Application

} // namespace Core

namespace Infrastructure {
    struct EventLogger
        : kgr::single_service<bxt::Infrastructure::EventLogger,
                              kgr::dependency<di::Utilities::EventBus>> {};

    struct WSController
        : kgr::shared_service<bxt::Infrastructure::WSController,
                              kgr::dependency<di::Utilities::EventBus>> {};

    struct PackageService
        : kgr::single_service<
              bxt::Infrastructure::PackageService,
              kgr::dependency<di::Core::Domain::PackageRepositoryBase>>,
          kgr::overrides<di::Core::Application::PackageService> {};

    struct DeploymentService
        : kgr::single_service<
              bxt::Infrastructure::DeploymentService,
              kgr::dependency<di::Core::Application::PackageService,
                              di::Core::Domain::ReadOnlySectionRepository>>,
          kgr::overrides<di::Core::Application::DeploymentService> {};

    struct ArchRepoOptions
        : kgr::single_service<bxt::Infrastructure::ArchRepoOptions> {};

    struct ArchRepoSyncService
        : kgr::single_service<
              bxt::Infrastructure::ArchRepoSyncService,
              kgr::dependency<di::Utilities::EventBusDispatcher,
                              di::Core::Domain::PackageRepositoryBase,
                              di::Infrastructure::ArchRepoOptions>>,
          kgr::overrides<di::Core::Application::SyncService> {};

} // namespace Infrastructure

namespace Persistence {
    struct UserRepository
        : kgr::single_service<
              bxt::Persistence::LMDB::UserRepository,
              kgr::dependency<di::Utilities::LMDB::Environment>>,
          kgr::overrides<di::Core::Domain::UserRepository> {};

    struct PackageLogEntryRepository
        : kgr::single_service<
              bxt::Infrastructure::DispatchingUnitOfWork<
                  bxt::Persistence::LMDB::PackageLogEntryRepository>,
              kgr::dependency<di::Utilities::LMDB::Environment>>,
          kgr::overrides<di::Core::Domain::PackageLogEntryRepository>,
          kgr::autocall<
              kgr::invoke<method<&bxt::Infrastructure::DispatchingUnitOfWork<
                              bxt::Persistence::LMDB::
                                  PackageLogEntryRepository>::init_dispatcher>,
                          di::Utilities::EventBusDispatcher>> {};

    struct SectionRepository
        : kgr::single_service<
              bxt::Persistence::SectionRepository,
              kgr::dependency<di::Utilities::RepoSchema::Parser>>,
          kgr::overrides<di::Core::Domain::ReadOnlySectionRepository> {};

    namespace Box {
        struct PoolOptions
            : kgr::single_service<bxt::Persistence::Box::PoolOptions> {};

        struct PoolBase
            : kgr::abstract_service<bxt::Persistence::Box::PoolBase> {};

        struct Pool
            : kgr::single_service<
                  bxt::Persistence::Box::Pool,
                  kgr::dependency<di::Persistence::Box::PoolOptions,
                                  di::Core::Domain::ReadOnlySectionRepository>>,
              kgr::overrides<PoolBase> {};

        struct PackageStoreBase
            : kgr::abstract_service<bxt::Persistence::Box::PackageStoreBase> {};

        struct LMDBPackageStore
            : kgr::single_service<
                  bxt::Persistence::Box::LMDBPackageStore,
                  kgr::dependency<di::Utilities::LMDB::Environment,
                                  di::Persistence::Box::PoolBase>>,
              kgr::overrides<PackageStoreBase> {};

        struct WritebackScheduler
            : kgr::single_service<bxt::Persistence::Box::WritebackScheduler,
                                  kgr::dependency<Utilities::IOScheduler>> {};

        struct ExporterBase
            : kgr::abstract_service<bxt::Persistence::Box::ExporterBase> {};

        struct AlpmDBExporter
            : kgr::single_service<
                  bxt::Persistence::Box::AlpmDBExporter,
                  kgr::dependency<PackageStoreBase,
                                  Core::Domain::ReadOnlySectionRepository>>,
              kgr::overrides<ExporterBase> {};

        struct BoxOptions
            : kgr::single_service<bxt::Persistence::Box::BoxOptions> {};

        struct BoxRepository
            : kgr::single_service<bxt::Infrastructure::DispatchingUnitOfWork<
                                      bxt::Persistence::Box::BoxRepository>,
                                  kgr::dependency<BoxOptions,
                                                  PackageStoreBase,
                                                  WritebackScheduler,
                                                  ExporterBase>>,
              kgr::overrides<di::Core::Domain::PackageRepositoryBase>,
              kgr::autocall<kgr::invoke<
                  method<&bxt::Infrastructure::DispatchingUnitOfWork<
                      bxt::Persistence::Box::BoxRepository>::init_dispatcher>,
                  di::Utilities::EventBusDispatcher>> {};
    } // namespace Box
} // namespace Persistence

namespace Presentation {

    struct PackageController
        : kgr::shared_service<
              bxt::Presentation::PackageController,
              kgr::dependency<di::Core::Application::PackageService,
                              di::Core::Application::SyncService,
                              di::Core::Application::PermissionService>> {};

    struct DeploymentController
        : kgr::shared_service<
              bxt::Presentation::DeploymentController,
              kgr::dependency<di::Core::Application::DeploymentService>> {};

    struct CompareController
        : kgr::shared_service<
              bxt::Presentation::CompareController,
              kgr::dependency<di::Core::Application::CompareService,
                              di::Core::Application::PermissionService,
                              di::Core::Application::SectionService>> {};

    struct AuthController
        : kgr::shared_service<
              bxt::Presentation::AuthController,
              kgr::dependency<di::Core::Application::AuthService>> {};

    struct UserController
        : kgr::shared_service<
              bxt::Presentation::UserController,
              kgr::dependency<di::Core::Application::UserService,
                              di::Core::Application::PermissionService>> {};

    struct LogController
        : kgr::shared_service<
              bxt::Presentation::LogController,
              kgr::dependency<di::Core::Application::PackageLogEntryService,
                              di::Core::Application::PermissionService>> {};

    struct SectionController
        : kgr::shared_service<
              bxt::Presentation::SectionController,
              kgr::dependency<di::Core::Application::SectionService,
                              di::Core::Application::PermissionService>> {};

    struct JwtFilter
        : kgr::shared_service<
              bxt::Presentation::JwtFilter,
              kgr::dependency<di::Core::Application::AuthService>> {};

} // namespace Presentation

} // namespace bxt::di
