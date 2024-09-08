/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageDTO.h"
#include "core/application/services/DeploymentService.h"
#include "core/application/services/PackageService.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "dexode/EventBus.hpp"
#include "PackageService.h"
#include "utilities/eventbus/EventBusDispatcher.h"
#include "utilities/StaticDTOMapper.h"

#include <cstdint>
#include <filesystem>
#include <set>
#include <vector>

namespace bxt::Infrastructure {

class DeploymentService : public bxt::Core::Application::DeploymentService {
public:
    DeploymentService(
        Utilities::EventBusDispatcher& dispatcher,
        bxt::Core::Application::PackageService& service,
        bxt::Core::Domain::ReadOnlyRepositoryBase<bxt::Core::Domain::Section>& section_repository,
        UnitOfWorkBaseFactory& uow_factory)
        : m_dispatcher(dispatcher)
        , m_package_service(service)
        , m_section_repository(section_repository)
        , m_uow_factory(uow_factory) {
    }

    virtual coro::task<Result<uint64_t>> deploy_start(RequestContext const context) override;
    virtual coro::task<Result<void>> deploy_push(PackageDTO package, uint64_t session_id) override;
    virtual coro::task<Result<void>> deploy_end(uint64_t session_id) override;

    virtual coro::task<Result<void>> verify_session(uint64_t session_id) override;

private:
    struct Session {
        std::vector<PackageDTO> packages;
        std::string run_id;
    };

    phmap::parallel_node_hash_map<uint64_t, Session> m_session_packages;
    Utilities::EventBusDispatcher& m_dispatcher;
    bxt::Core::Application::PackageService& m_package_service;
    bxt::Core::Domain::ReadOnlyRepositoryBase<bxt::Core::Domain::Section>& m_section_repository;
    UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::Infrastructure
