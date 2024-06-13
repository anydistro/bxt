/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageLogEntryDTO.h"
#include "core/domain/entities/PackageLogEntry.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "dexode/EventBus.hpp"

#include <memory>

namespace bxt::Core::Application {

class PackageLogEntryService {
public:
    PackageLogEntryService(
        std::shared_ptr<dexode::EventBus> evbus,
        bxt::Core::Domain::ReadWriteRepositoryBase<Domain::PackageLogEntry>&
            repository,
        UnitOfWorkBaseFactory& uow_factory)
        : m_evbus(evbus),
          m_listener(dexode::EventBus::Listener::createNotOwning(*m_evbus)),
          m_repository(repository),
          m_uow_factory(uow_factory) {}

    void init();

    coro::task<std::vector<PackageLogEntryDTO>> events();

private:
    std::shared_ptr<dexode::EventBus> m_evbus;
    dexode::EventBus::Listener m_listener;
    bxt::Core::Domain::ReadWriteRepositoryBase<Domain::PackageLogEntry>&
        m_repository;
    UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::Core::Application
