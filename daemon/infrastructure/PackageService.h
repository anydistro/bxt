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
#include "coro/task.hpp"

#include <vector>
namespace bxt::Infrastructure {

class PackageService : public Core::Application::PackageService {
public:
    PackageService(Core::Domain::PackageRepositoryBase& repository)
        : m_repository(repository) {}

    virtual coro::task<Result<void>>
        commit_transaction(const Transaction transaction) override;

    virtual coro::task<Result<void>>
        add_package(const PackageDTO package) override;

    virtual coro::task<Result<std::vector<PackageDTO>>>
        get_packages(const PackageSectionDTO section_dto) const override;

    virtual coro::task<Result<void>>
        snap(const PackageSectionDTO from_section,
             const PackageSectionDTO to_section) override;

private:
    PackageServiceOptions m_options;
    Core::Domain::PackageRepositoryBase& m_repository;
};

} // namespace bxt::Infrastructure
