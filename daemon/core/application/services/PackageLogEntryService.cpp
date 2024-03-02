/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "PackageLogEntryService.h"

#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageLogEntryDTO.h"
#include "core/application/events/SyncEvent.h"
#include "core/domain/events/PackageEvents.h"

namespace bxt::Core::Application {

void PackageLogEntryService::init() {
    m_listener.listen<bxt::Core::Application::Events::SyncFinished>(
        [this](const Application::Events::SyncFinished &sync_event) {
            for (const auto &synced_package : sync_event.packages_synced) {
                m_repository.add(Domain::PackageLogEntry {
                    synced_package, Domain::LogEntryType::Add});
            }

            coro::sync_wait(m_repository.commit_async());
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

            m_repository.add(entry);
            coro::sync_wait(m_repository.commit_async());
        });
}

coro::task<std::vector<PackageLogEntryDTO>> PackageLogEntryService::events() {
    std::vector<PackageLogEntryDTO> result;

    const auto entities = co_await m_repository.all_async();

    if (!entities.has_value()) { co_return result; }

    result.reserve(entities->size());
    std::ranges::transform(*entities, std::back_inserter(result),
                           PackageLogEntryDTOMapper::to_dto);

    co_return result;
}

} // namespace bxt::Core::Application
