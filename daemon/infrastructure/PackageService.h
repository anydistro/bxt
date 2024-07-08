/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "PackageServiceOptions.h"
#include "core/application/dtos/PackageDTO.h"
#include "core/application/services/PackageService.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "coro/task.hpp"
#include "utilities/eventbus/EventBusDispatcher.h"

#include <memory>
#include <vector>
namespace bxt::Infrastructure {

class PackageService : public Core::Application::PackageService {
public:
    PackageService(Utilities::EventBusDispatcher& dispatcher,
                   Core::Domain::PackageRepositoryBase& repository,
                   UnitOfWorkBaseFactory& uow_factory)
        : m_dispatcher(dispatcher),
          m_repository(repository),
          m_uow_factory(uow_factory) {}

    virtual coro::task<Result<void>>
        commit_transaction(const Transaction transaction) override;

    virtual coro::task<Result<std::vector<PackageDTO>>>
        get_packages(const PackageSectionDTO section_dto) const override;

    virtual coro::task<Result<void>>
        snap(const PackageSectionDTO from_section,
             const PackageSectionDTO to_section) override;

    coro::task<Result<void>> push(const Transaction transaction,
                                  const RequestContext context) override;

private:
    coro::task<Result<void>> add_package(const PackageDTO package,
                                         std::shared_ptr<UnitOfWorkBase> uow);

    coro::task<PackageService::Result<void>>
        move_package(const std::string package_name,
                     const PackageSectionDTO from_section,
                     const PackageSectionDTO to_section,
                     std::shared_ptr<UnitOfWorkBase> unitofwork);

    coro::task<PackageService::Result<void>>
        copy_package(const std::string package_name,
                     const PackageSectionDTO from_section,
                     const PackageSectionDTO to_section,
                     std::shared_ptr<UnitOfWorkBase> unitofwork);

    PackageServiceOptions m_options;
    Utilities::EventBusDispatcher& m_dispatcher;
    Core::Domain::PackageRepositoryBase& m_repository;
    UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::Infrastructure
