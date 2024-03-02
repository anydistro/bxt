/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/entities/Package.h"
#include "core/domain/enums/ArchitectureMatch.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "core/domain/repositories/RepositoryBase.h"

#include <coro/task.hpp>
#include <dexode/EventBus.hpp>
#include <vector>

namespace bxt::Core::Domain {

class PackageService {
public:
    PackageService(std::shared_ptr<dexode::EventBus> evbus,
                   PackageRepositoryBase& repository)
        : m_evbus(evbus), m_repository(repository) {}

    coro::task<void> add_package(Package pkg);

    coro::task<void> remove_package(Package pkg);

    coro::task<void> update_package(Package pkg, bool enable_downgrade = false);

private:
    std::shared_ptr<dexode::EventBus> m_evbus;
    PackageRepositoryBase& m_repository;
};

} // namespace bxt::Core::Domain
