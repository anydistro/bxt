/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/services/AuthService.h"
#include "core/application/services/CompareService.h"
#include "core/application/services/PackageService.h"
#include "core/application/services/PermissionService.h"
#include "core/application/services/SectionService.h"
#include "core/application/services/UserService.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "coro/io_scheduler.hpp"
#include "event_log/application/services/LogService.h"
#include "event_log/domain/entities/CommitLogEntry.h"
#include "event_log/domain/entities/DeployLogEntry.h"
#include "event_log/domain/entities/SyncLogEntry.h"
#include "infrastructure/alpm/ArchRepoOptions.h"
#include "infrastructure/alpm/ArchRepoSyncService.h"
#include "infrastructure/DeploymentService.h"
#include "infrastructure/PackageService.h"
#include "infrastructure/ws/WSController.h"
#include "kangaru/autowire.hpp"
#include "kangaru/detail/single.hpp"
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
#include "persistence/lmdb/LmdbUnitOfWork.h"
#include "persistence/lmdb/LogEntryRepositories.h"
#include "persistence/lmdb/UserRepository.h"
#include "presentation/cli-controllers/DeploymentController.h"
#include "presentation/cli-controllers/DeploymentOptions.h"
#include "presentation/JwtOptions.h"
#include "presentation/web-controllers/AuthController.h"
#include "presentation/web-controllers/CompareController.h"
#include "presentation/web-controllers/LogController.h"
#include "presentation/web-controllers/PackageController.h"
#include "presentation/web-controllers/SectionController.h"
#include "presentation/web-controllers/UserController.h"
#include "presentation/web-filters/JwtFilter.h"
#include "utilities/configuration/Configuration.h"
#include "utilities/lmdb/Environment.h"
#include "utilities/lmdb/LMDBOptions.h"
#include "utilities/repo-schema/Parser.h"

#include <infrastructure/EventLogger.h>
#include <kangaru/autocall.hpp>
#include <kangaru/debug.hpp>
#include <kangaru/kangaru.hpp>
#include <kangaru/operator_service.hpp>

template<auto m> using method = kgr::method<decltype(m), m>;

namespace bxt::di {

namespace Utilities {

    struct EventBus : kgr::shared_service<dexode::EventBus> {};

    struct EventBusDispatcher
        : kgr::single_service<bxt::Utilities::EventBusDispatcher,
                              kgr::dependency<di::Utilities::EventBus>> {};

    struct IOScheduler : kgr::extern_shared_service<coro::io_scheduler> {};

    namespace LMDB {
        struct LMDBOptions : kgr::single_service<bxt::Utilities::LMDB::LMDBOptions> {};

        struct Environment
            : kgr::shared_service<bxt::Utilities::LMDB::Environment, kgr::dependency<IOScheduler>> {
        };

    } // namespace LMDB

    namespace RepoSchema {

        struct Parser : kgr::autowire_single_service<bxt::Utilities::RepoSchema::Parser> {};

    } // namespace RepoSchema

    struct Configuration : kgr::autowire_single_service<bxt::Utilities::Configuration> {};

} // namespace Utilities

namespace Core {

    namespace Domain {

        struct UserRepository
            : kgr::abstract_service<bxt::Core::Domain::ReadWriteRepositoryBase<User>> {};

        struct PackageRepositoryBase
            : kgr::abstract_service<bxt::Core::Domain::PackageRepositoryBase> {};

        struct ReadOnlySectionRepository
            : kgr::abstract_service<bxt::Core::Domain::ReadOnlyRepositoryBase<Section>> {};

        struct UnitOfWorkBaseFactory
            : kgr::abstract_service<bxt::Core::Domain::UnitOfWorkBaseFactory> {};

    } // namespace Domain

    namespace Application {

        struct AuthService
            : kgr::single_service<bxt::Core::Application::AuthService,
                                  kgr::dependency<di::Core::Domain::UserRepository,
                                                  di::Core::Domain::UnitOfWorkBaseFactory>> {};

        struct DeploymentService
            : kgr::abstract_service<bxt::Core::Application::DeploymentService> {};

        struct PackageService : kgr::abstract_service<bxt::Core::Application::PackageService> {};

        struct SyncService : kgr::abstract_service<bxt::Core::Application::SyncService> {};

        struct UserService
            : kgr::single_service<bxt::Core::Application::UserService,
                                  kgr::dependency<di::Core::Domain::UserRepository,
                                                  di::Core::Domain::UnitOfWorkBaseFactory>> {};

        struct PermissionService
            : kgr::single_service<bxt::Core::Application::PermissionService,
                                  kgr::dependency<di::Core::Domain::UserRepository,
                                                  di::Core::Domain::UnitOfWorkBaseFactory>> {};

        struct SectionService
            : kgr::single_service<bxt::Core::Application::SectionService,
                                  kgr::dependency<di::Core::Domain::ReadOnlySectionRepository,
                                                  di::Core::Domain::UnitOfWorkBaseFactory>> {};

        struct CompareService
            : kgr::single_service<bxt::Core::Application::CompareService,
                                  kgr::dependency<di::Core::Application::PackageService>> {};

    } // namespace Application

} // namespace Core

namespace EventLog {

    namespace Domain {

        struct SyncLogEntryRepository
            : kgr::abstract_service<
                  bxt::Core::Domain::ReadWriteRepositoryBase<bxt::EventLog::Domain::SyncLogEntry>> {
        };
        struct CommitLogEntryRepository
            : kgr::abstract_service<bxt::Core::Domain::ReadWriteRepositoryBase<
                  bxt::EventLog::Domain::CommitLogEntry>> {};
        struct DeployLogEntryRepository
            : kgr::abstract_service<bxt::Core::Domain::ReadWriteRepositoryBase<
                  bxt::EventLog::Domain::DeployLogEntry>> {};

    } // namespace Domain
    namespace Application {

        struct PackageLogEntryService
            : kgr::single_service<bxt::EventLog::Application::LogService,
                                  kgr::dependency<di::Utilities::EventBus,
                                                  Domain::SyncLogEntryRepository,
                                                  Domain::CommitLogEntryRepository,
                                                  Domain::DeployLogEntryRepository,
                                                  di::Core::Domain::UnitOfWorkBaseFactory>>
            , kgr::autocall<kgr::invoke<method<&bxt::EventLog::Application::LogService::init>>> {};

    } // namespace Application

} // namespace EventLog
namespace Infrastructure {
    struct EventLogger
        : kgr::single_service<bxt::Infrastructure::EventLogger,
                              kgr::dependency<di::Utilities::EventBus>> {};

    struct WSController
        : kgr::shared_service<bxt::Infrastructure::WSController,
                              kgr::dependency<di::Utilities::EventBus>> {};

    struct PackageService
        : kgr::single_service<bxt::Infrastructure::PackageService,
                              kgr::dependency<di::Utilities::EventBusDispatcher,
                                              di::Core::Domain::PackageRepositoryBase,
                                              di::Core::Domain::ReadOnlySectionRepository,
                                              di::Core::Domain::UnitOfWorkBaseFactory>>
        , kgr::overrides<di::Core::Application::PackageService> {};

    struct DeploymentService
        : kgr::single_service<bxt::Infrastructure::DeploymentService,
                              kgr::dependency<di::Utilities::EventBusDispatcher,
                                              di::Core::Application::PackageService,
                                              di::Core::Domain::ReadOnlySectionRepository,
                                              di::Core::Domain::UnitOfWorkBaseFactory>>
        , kgr::overrides<di::Core::Application::DeploymentService> {};

    struct ArchRepoOptions : kgr::single_service<bxt::Infrastructure::ArchRepoOptions> {};

    struct ArchRepoSyncService
        : kgr::single_service<bxt::Infrastructure::ArchRepoSyncService,
                              kgr::dependency<di::Utilities::EventBusDispatcher,
                                              di::Core::Domain::PackageRepositoryBase,
                                              di::Infrastructure::ArchRepoOptions,
                                              di::Core::Domain::UnitOfWorkBaseFactory>>
        , kgr::overrides<di::Core::Application::SyncService> {};

} // namespace Infrastructure

namespace Persistence {

    struct UnitOfWorkFactory
        : kgr::single_service<bxt::Persistence::LmdbUnitOfWorkFactory,
                              kgr::dependency<di::Utilities::LMDB::Environment>>
        , kgr::overrides<di::Core::Domain::UnitOfWorkBaseFactory> {};

    struct UserRepository
        : kgr::single_service<bxt::Persistence::LMDB::UserRepository,
                              kgr::dependency<di::Utilities::LMDB::Environment>>
        , kgr::overrides<di::Core::Domain::UserRepository> {};

    struct SyncLogEntryRepository
        : kgr::single_service<bxt::Persistence::LMDB::SyncLogEntryRepository,
                              kgr::dependency<di::Utilities::LMDB::Environment>>
        , kgr::overrides<di::EventLog::Domain::SyncLogEntryRepository> {};

    struct DeployLogEntryRepository
        : kgr::single_service<bxt::Persistence::LMDB::DeployLogEntryRepository,
                              kgr::dependency<di::Utilities::LMDB::Environment>>
        , kgr::overrides<di::EventLog::Domain::DeployLogEntryRepository> {};

    struct CommitLogEntryRepository
        : kgr::single_service<bxt::Persistence::LMDB::CommitLogEntryRepository,
                              kgr::dependency<di::Utilities::LMDB::Environment>>
        , kgr::overrides<di::EventLog::Domain::CommitLogEntryRepository> {};

    struct SectionRepository
        : kgr::single_service<bxt::Persistence::SectionRepository,
                              kgr::dependency<di::Utilities::RepoSchema::Parser>>
        , kgr::overrides<di::Core::Domain::ReadOnlySectionRepository> {};

    namespace Box {
        struct PoolOptions : kgr::single_service<bxt::Persistence::Box::PoolOptions> {};

        struct BoxOptions : kgr::single_service<bxt::Persistence::Box::BoxOptions> {};

        struct PoolBase : kgr::abstract_service<bxt::Persistence::Box::PoolBase> {};

        struct Pool
            : kgr::single_service<bxt::Persistence::Box::Pool,
                                  kgr::dependency<di::Persistence::Box::BoxOptions,
                                                  di::Persistence::Box::PoolOptions,
                                                  di::Core::Domain::ReadOnlySectionRepository,
                                                  di::Core::Domain::UnitOfWorkBaseFactory>>
            , kgr::overrides<PoolBase> {};

        struct PackageStoreBase : kgr::abstract_service<bxt::Persistence::Box::PackageStoreBase> {};

        struct LMDBPackageStore
            : kgr::single_service<bxt::Persistence::Box::LMDBPackageStore,
                                  kgr::dependency<di::Persistence::Box::BoxOptions,
                                                  di::Utilities::LMDB::Environment,
                                                  di::Persistence::Box::PoolBase,
                                                  di::Core::Domain::ReadOnlySectionRepository>>
            , kgr::overrides<PackageStoreBase> {};

        struct WritebackScheduler
            : kgr::single_service<bxt::Persistence::Box::WritebackScheduler,
                                  kgr::dependency<Utilities::IOScheduler>> {};

        struct ExporterBase : kgr::abstract_service<bxt::Persistence::Box::ExporterBase> {};

        struct AlpmDBExporter
            : kgr::single_service<bxt::Persistence::Box::AlpmDBExporter,
                                  kgr::dependency<di::Persistence::Box::BoxOptions,
                                                  di::Persistence::Box::PackageStoreBase,
                                                  Core::Domain::ReadOnlySectionRepository,
                                                  di::Core::Domain::UnitOfWorkBaseFactory>>
            , kgr::overrides<ExporterBase> {};

        struct BoxRepository
            : kgr::single_service<
                  bxt::Persistence::Box::BoxRepository,
                  kgr::dependency<BoxOptions, PackageStoreBase, WritebackScheduler, ExporterBase>>
            , kgr::overrides<di::Core::Domain::PackageRepositoryBase> {};
    } // namespace Box
} // namespace Persistence

namespace Presentation {

    struct JwtOptions : kgr::single_service<bxt::Presentation::JwtOptions> {};

    struct PackageController
        : kgr::shared_service<bxt::Presentation::PackageController,
                              kgr::dependency<di::Core::Application::PackageService,
                                              di::Core::Application::SyncService,
                                              di::Core::Application::PermissionService>> {};

    struct DeploymentOptions : kgr::single_service<bxt::Presentation::DeploymentOptions> {};

    struct DeploymentController
        : kgr::shared_service<bxt::Presentation::DeploymentController,
                              kgr::dependency<di::Presentation::DeploymentOptions,
                                              di::Core::Application::DeploymentService>> {};

    struct CompareController
        : kgr::shared_service<bxt::Presentation::CompareController,
                              kgr::dependency<di::Core::Application::CompareService,
                                              di::Core::Application::PermissionService,
                                              di::Core::Application::SectionService>> {};

    struct AuthController
        : kgr::shared_service<bxt::Presentation::AuthController,
                              kgr::dependency<di::Presentation::JwtOptions,
                                              di::Core::Application::AuthService,
                                              di::Utilities::LMDB::Environment,
                                              di::Core::Domain::UnitOfWorkBaseFactory>> {};

    struct UserController
        : kgr::shared_service<bxt::Presentation::UserController,
                              kgr::dependency<di::Core::Application::UserService,
                                              di::Core::Application::PermissionService>> {};

    struct LogController
        : kgr::shared_service<bxt::Presentation::LogController,
                              kgr::dependency<di::EventLog::Application::PackageLogEntryService,
                                              di::Core::Application::PermissionService>> {};

    struct SectionController
        : kgr::shared_service<bxt::Presentation::SectionController,
                              kgr::dependency<di::Core::Application::SectionService,
                                              di::Core::Application::PermissionService>> {};

    struct JwtFilter
        : kgr::shared_service<
              bxt::Presentation::JwtFilter,
              kgr::dependency<di::Presentation::JwtOptions, di::Core::Application::AuthService>> {};

} // namespace Presentation

} // namespace bxt::di
