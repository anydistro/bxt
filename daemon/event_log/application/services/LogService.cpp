/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "LogService.h"

#include "core/application/events/CommitEvent.h"
#include "core/application/events/DeployEvent.h"
#include "core/application/events/SyncEvent.h"
#include "core/application/services/PackageService.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "event_log/application/dtos/CommitLogEntryDTO.h"
#include "event_log/application/dtos/SyncLogEntryDTO.h"
#include "event_log/domain/entities/PackageLogEntry.h"
#include "event_log/domain/entities/PackageUpdateLogEntry.h"
#include "event_log/domain/entities/SyncLogEntry.h"
#include "event_log/domain/enums/LogEntryType.h"
#include "utilities/StaticDTOMapper.h"
#include "utilities/to_string.h"

#include <fmt/chrono.h>
#include <ranges>
#include <type_traits>

namespace bxt::EventLog::Application {
auto pkg_to_log_entry(const bxt::Core::Domain::Package &pkg) {
    return bxt::EventLog::Domain::PackageLogEntry(
        bxt::EventLog::Domain::LogEntryType::Add, pkg.section(), pkg.name(),
        pkg.location(), pkg.version());
}
auto transfer_action_to_update_log_entry(
    const Core::Application::PackageService::Transaction::TransferAction
        &action) {
    return Domain::PackageUpdateLogEntry(
        Domain::PackageLogEntry {Domain::LogEntryType::Update,
                                 SectionDTOMapper::to_entity(action.to_section),
                                 action.name, PoolLocation::Overlay

        },
        Domain::PackageLogEntry {
            Domain::LogEntryType::Update,
            SectionDTOMapper::to_entity(action.from_section), action.name,
            PoolLocation::Overlay});
}

void LogService::init() {
    using std::ranges::to;
    using std::views::transform;

    using namespace bxt::Core::Application::Events;

    m_listener.listen<SyncFinished>([this](const SyncFinished &sync_event) {
        Domain::SyncLogEntry sync_log_entry(
            sync_event.when, sync_event.user_name,
            sync_event.added_packages | transform(pkg_to_log_entry)
                | to<std::vector>(),
            sync_event.deleted_package_ids
                | transform([](const Package::TId &id) {
                      return Domain::PackageLogEntry {
                          Domain::LogEntryType::Remove, id.section,
                          id.package_name, PoolLocation::Sync};
                  })
                | to<std::vector>());

        coro::sync_wait([this, sync_log_entry = std::move(
                                   sync_log_entry)]() -> coro::task<void> {
            auto uow = co_await m_uow_factory(true);
            auto saved =
                co_await m_sync_repository.save_async(sync_log_entry, uow);

            auto commited = co_await uow->commit_async();

            co_return;
        }());
    });

    m_listener.listen<Commited>([this](const auto &commit_event) {
        Domain::CommitLogEntry commit_log_entry {
            commit_event.when,
            commit_event.user_name,
            commit_event.to_add | transform(pkg_to_log_entry)
                | to<std::vector>(),
            commit_event.to_delete | transform([](const Package::TId &id) {
                return Domain::PackageLogEntry {Domain::LogEntryType::Remove,
                                                id.section, id.package_name,
                                                PoolLocation::Overlay};
            }) | to<std::vector>(),
            commit_event.to_move
                | transform(transfer_action_to_update_log_entry)
                | to<std::vector>(),
            commit_event.to_copy
                | transform(transfer_action_to_update_log_entry)
                | to<std::vector>()};

        coro::sync_wait([this, commit_log_entry = std::move(
                                   commit_log_entry)]() -> coro::task<void> {
            auto uow = co_await m_uow_factory(true);
            auto saved =
                co_await m_commit_repository.save_async(commit_log_entry, uow);

            auto committed = co_await uow->commit_async();

            co_return;
        }());
    });

    m_listener.listen<DeploySuccess>([this](const auto &deploy_event) {
        Domain::DeployLogEntry deploy_log_entry {
            deploy_event.when, deploy_event.deployment_url,
            deploy_event.added_packages | transform(pkg_to_log_entry)
                | to<std::vector>()};
        coro::sync_wait([this, deploy_log_entry = std::move(
                                   deploy_log_entry)]() -> coro::task<void> {
            auto uow = co_await m_uow_factory(true);
            auto saved =
                co_await m_deploy_repository.save_async(deploy_log_entry, uow);
            auto committed = co_await uow->commit_async();
            co_return;
        }());
    });
}

auto package_entry_filter(const Domain::PackageLogEntry &entry,
                          const std::string &full_text) {
    if (bxt::to_string(entry.location()).contains(full_text)) { return true; }
    if (bxt::to_string(entry.section()).contains(full_text)) { return true; }
    if (entry.name().contains(full_text)) { return true; }

    if (entry.version()
        && bxt::to_string(*entry.version()).contains(full_text)) {
        return true;
    }

    return false;
};
coro::task<LogService::LogEntriesDTO>
    LogService::events(const LogService::EventSpecification spec) {
    auto uow = co_await m_uow_factory();

    const auto [since, until, full_text] = spec;

    std::decay_t<decltype(m_sync_repository)>::TResults sync_entities;
    std::decay_t<decltype(m_commit_repository)>::TResults commit_entities;
    std::decay_t<decltype(m_deploy_repository)>::TResults deploy_entities;

    const auto time_filter = [since, until](const auto &entry) {
        return entry.time() >= since && entry.time() <= until;
    };

    if (!full_text.has_value()) {
        sync_entities = co_await m_sync_repository.find_async(time_filter, uow);
        commit_entities =
            co_await m_commit_repository.find_async(time_filter, uow);
        deploy_entities =
            co_await m_deploy_repository.find_async(time_filter, uow);
    } else {
        sync_entities = co_await m_sync_repository.find_async(
            [since, until, full_text = *full_text,
             time_filter](const auto &entry) {
                if (!time_filter(entry)) { return false; }

                if (entry.sync_trigger_username().contains(full_text)) {
                    return true;
                }

                for (const auto &package : entry.added()) {
                    if (package_entry_filter(package, full_text)) {
                        return true;
                    }
                }
                for (const auto &package : entry.deleted()) {
                    if (package_entry_filter(package, full_text)) {
                        return true;
                    }
                }

                return false;
            },
            uow);
        commit_entities = co_await m_commit_repository.find_async(
            [since, until, full_text = *spec.full_text,
             time_filter](const auto &entry) {
                if (!time_filter(entry)) { return false; }

                if (entry.commiter_name().contains(full_text)) { return true; }

                for (const auto &package : entry.added()) {
                    if (package_entry_filter(package, full_text)) {
                        return true;
                    }
                }
                for (const auto &package : entry.deleted()) {
                    if (package_entry_filter(package, full_text)) {
                        return true;
                    }
                }
                for (const auto &package : entry.moved()) {
                    if (package_entry_filter(package.package(), full_text)
                        || package_entry_filter(package.previous_package(),
                                                full_text)) {
                        return true;
                    }
                }
                for (const auto &package : entry.copied()) {
                    if (package_entry_filter(package.package(), full_text)
                        || package_entry_filter(package.previous_package(),
                                                full_text)) {
                        return true;
                    }
                }
                return false;
            },
            uow);

        deploy_entities = co_await m_deploy_repository.find_async(
            [since, until, full_text = *spec.full_text,
             time_filter](const auto &entry) {
                if (!time_filter(entry)) { return false; }

                if (entry.runner_url().contains(full_text)) { return true; }

                for (const auto &package : entry.added()) {
                    if (package_entry_filter(package, full_text)) {
                        return true;
                    }
                }
                return false;
            },
            uow);
    }

    if (!sync_entities) {
        sync_entities = std::vector<Domain::SyncLogEntry> {};
    }
    if (!commit_entities) {
        commit_entities = std::vector<Domain::CommitLogEntry> {};
    }
    if (!deploy_entities) {
        deploy_entities = std::vector<Domain::DeployLogEntry> {};
    }

    co_return LogEntriesDTO {
        .commits = Utilities::map_entries(*commit_entities,
                                          CommitLogEntryDTOMapper::to_dto),
        .deploys = Utilities::map_entries(*deploy_entities,
                                          DeployLogEntryDTOMapper::to_dto),
        .syncs = Utilities::map_entries(*sync_entities,
                                        SyncLogEntryDTOMapper::to_dto)};
}
} // namespace bxt::EventLog::Application
