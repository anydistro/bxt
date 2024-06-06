/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "PackageService.h"
#include "core/application/dtos/PackageDTO.h"
#include "core/application/services/DeploymentService.h"
#include "core/application/services/PackageService.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "utilities/StaticDTOMapper.h"

#include <cstdint>
#include <filesystem>
#include <set>
#include <vector>

namespace bxt::Infrastructure {

class DeploymentService : public bxt::Core::Application::DeploymentService {
public:
    DeploymentService(
        bxt::Core::Application::PackageService& service,
        bxt::Core::Domain::ReadOnlyRepositoryBase<bxt::Core::Domain::Section>&
            section_repository,
        UnitOfWorkBaseFactory& uow_factory)
        : m_package_service(service),
          m_section_repository(section_repository),
          m_uow_factory(uow_factory) {}

    virtual coro::task<Result<uint64_t>> deploy_start() override;
    virtual coro::task<Result<void>> deploy_push(PackageDTO package,
                                                 uint64_t session_id) override;
    virtual coro::task<Result<void>> deploy_end(uint64_t session_id) override;

    virtual coro::task<Result<void>>
        verify_session(uint64_t session_id) override;

private:
    phmap::parallel_node_hash_map<uint64_t, std::vector<PackageDTO>>
        m_session_packages;
    bxt::Core::Application::PackageService& m_package_service;
    bxt::Core::Domain::ReadOnlyRepositoryBase<bxt::Core::Domain::Section>&
        m_section_repository;
    UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::Infrastructure
