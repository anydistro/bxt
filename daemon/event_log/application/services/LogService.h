/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "event_log/application/dtos/CommitLogEntryDTO.h"
#include "event_log/application/dtos/DeployLogEntryDTO.h"
#include "event_log/application/dtos/SyncLogEntryDTO.h"
#include "event_log/domain/entities/CommitLogEntry.h"
#include "event_log/domain/entities/DeployLogEntry.h"
#include "event_log/domain/entities/SyncLogEntry.h"

#include <dexode/EventBus.hpp>
#include <memory>
#include <optional>
#include <utility>

namespace bxt::EventLog::Application {

class LogService {
public:
    LogService(
        std::shared_ptr<dexode::EventBus> evbus,
        bxt::Core::Domain::ReadWriteRepositoryBase<Domain::SyncLogEntry>& sync_repository,
        bxt::Core::Domain::ReadWriteRepositoryBase<Domain::CommitLogEntry>& commit_repository,
        bxt::Core::Domain::ReadWriteRepositoryBase<Domain::DeployLogEntry>& deploy_repository,
        UnitOfWorkBaseFactory& uow_factory)
        : m_evbus(std::move(evbus))
        , m_listener(dexode::EventBus::Listener::createNotOwning(*m_evbus))
        , m_sync_repository(sync_repository)
        , m_commit_repository(commit_repository)
        , m_deploy_repository(deploy_repository)
        , m_uow_factory(uow_factory) {
    }

    void init();

    struct LogEntriesDTO {
        std::vector<CommitLogEntryDTO> commits;
        std::vector<DeployLogEntryDTO> deploys;
        std::vector<SyncLogEntryDTO> syncs;
    };

    struct EventSpecification {
        std::chrono::system_clock::time_point since;
        std::chrono::system_clock::time_point until;
        std::optional<std::string> full_text;
    };

    coro::task<LogEntriesDTO> events(EventSpecification const spec);

private:
    std::shared_ptr<dexode::EventBus> m_evbus;
    dexode::EventBus::Listener m_listener;

    bxt::Core::Domain::ReadWriteRepositoryBase<Domain::SyncLogEntry>& m_sync_repository;
    bxt::Core::Domain::ReadWriteRepositoryBase<Domain::CommitLogEntry>& m_commit_repository;
    bxt::Core::Domain::ReadWriteRepositoryBase<Domain::DeployLogEntry>& m_deploy_repository;
    UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::EventLog::Application
