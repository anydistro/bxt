/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PackageLogEntryService.h"

#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageLogEntryDTO.h"
#include "core/application/events/SyncEvent.h"
#include "core/domain/events/PackageEvents.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "coro/sync_wait.hpp"
#include "utilities/log/Logging.h"

namespace bxt::Core::Application {

void PackageLogEntryService::init() {
    m_listener.listen<bxt::Core::Application::Events::SyncFinished>(
        [this](const Application::Events::SyncFinished &sync_event) {
            for (const auto &package : sync_event.packages_synced) {
                Domain::PackageLogEntry entry(package,
                                              Domain::LogEntryType::Add);
                coro::sync_wait(
                    [this, entry = std::move(entry)]() -> coro::task<void> {
                        auto uow = co_await m_uow_factory();

                        auto add_ok =
                            co_await m_repository.add_async(entry, uow);
                        if (!add_ok.has_value()) {
                            loge("Failed to add package log entry");
                            co_return;
                        }

                        auto commit_ok = co_await uow->commit_async();
                        if (!commit_ok.has_value()) {
                            loge("Failed to commit transaction");
                        }
                    }());
            }
        });

    //    listener.listen<bxt::Core::Domain::Events::PackageRemoved>(
    //        [this](const Domain::Events::PackageRemoved &added) {
    //            Domain::PackageLogEntry entry(added.id,
    //            Domain::LogEntryType::Add);

    //            m_repository.add(entry);
    //        });
    m_listener.listen<bxt::Core::Domain::Events::PackageUpdated>(
        [this](const Domain::Events::PackageUpdated &added) {
            Domain::PackageLogEntry entry(added.new_package,
                                          Domain::LogEntryType::Add);
            coro::sync_wait(
                [this, entry = std::move(entry)]() -> coro::task<void> {
                    auto uow = co_await m_uow_factory();

                    auto add_ok = co_await m_repository.add_async(entry, uow);
                    if (!add_ok.has_value()) {
                        loge("Failed to add package log entry");
                        co_return;
                    }

                    auto commit_ok = co_await uow->commit_async();
                    if (!commit_ok.has_value()) {
                        loge("Failed to commit transaction");
                    }
                }());
        });
}

coro::task<std::vector<PackageLogEntryDTO>> PackageLogEntryService::events() {
    std::vector<PackageLogEntryDTO> result;

    const auto entities =
        co_await m_repository.all_async(co_await m_uow_factory());

    if (!entities.has_value()) { co_return result; }

    result.reserve(entities->size());
    std::ranges::transform(*entities, std::back_inserter(result),
                           PackageLogEntryDTOMapper::to_dto);

    co_return result;
}

} // namespace bxt::Core::Application
