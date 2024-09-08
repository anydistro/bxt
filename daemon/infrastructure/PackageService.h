/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/dtos/PackageDTO.h"
#include "core/application/services/PackageService.h"
#include "core/domain/entities/Section.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "core/domain/repositories/ReadOnlyRepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "coro/task.hpp"
#include "PackageServiceOptions.h"
#include "utilities/eventbus/EventBusDispatcher.h"

#include <memory>
#include <vector>
namespace bxt::Infrastructure {

class PackageService : public Core::Application::PackageService {
public:
    PackageService(Utilities::EventBusDispatcher& dispatcher,
                   Core::Domain::PackageRepositoryBase& repository,
                   Core::Domain::ReadOnlyRepositoryBase<Core::Domain::Section>& section_repository,
                   UnitOfWorkBaseFactory& uow_factory)
        : m_dispatcher(dispatcher)
        , m_repository(repository)
        , m_section_repository(section_repository)
        , m_uow_factory(uow_factory) {
    }

    virtual coro::task<Result<void>> commit_transaction(Transaction const transaction) override;

    virtual coro::task<Result<std::vector<PackageDTO>>>
        get_packages(PackageSectionDTO const section_dto) const override;

    virtual coro::task<Result<void>> snap(PackageSectionDTO const from_section,
                                          PackageSectionDTO const to_section) override;

    coro::task<Result<void>> push(Transaction const transaction,
                                  RequestContext const context) override;

    coro::task<Result<void>> snap_branch(std::string const from_branch,
                                         std::string const to_branch,
                                         std::string const arch) override;

private:
    coro::task<Result<void>> add_package(PackageDTO const package,
                                         std::shared_ptr<UnitOfWorkBase> uow);

    coro::task<PackageService::Result<void>>
        move_package(std::string const package_name,
                     PackageSectionDTO const from_section,
                     PackageSectionDTO const to_section,
                     std::shared_ptr<UnitOfWorkBase> unitofwork);

    coro::task<PackageService::Result<void>>
        copy_package(std::string const package_name,
                     PackageSectionDTO const from_section,
                     PackageSectionDTO const to_section,
                     std::shared_ptr<UnitOfWorkBase> unitofwork);

    PackageServiceOptions m_options;
    Utilities::EventBusDispatcher& m_dispatcher;
    Core::Domain::PackageRepositoryBase& m_repository;
    Core::Domain::ReadOnlyRepositoryBase<Section>& m_section_repository;
    UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::Infrastructure
