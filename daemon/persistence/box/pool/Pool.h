/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "PoolOptions.h"
#include "core/domain/enums/PoolLocation.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "persistence/box/BoxOptions.h"
#include "persistence/box/pool/PoolBase.h"

#include <filesystem>
#include <string>

namespace bxt::Persistence::Box {
class PackageRecord;

class Pool : public PoolBase {
public:
    Pool(BoxOptions& box_options,
         PoolOptions& options,
         ReadOnlyRepositoryBase<Section>& section_repository,
         UnitOfWorkBaseFactory& uow_factory);

    PoolBase::Result<PackageRecord>
        move_to(const PackageRecord& package) override;

private:
    std::string
        format_target_path(Core::Domain::PoolLocation location,
                           const std::string& arch,
                           const std::optional<std::string>& filename = {});

    std::filesystem::path m_pool_path;
    std::set<std::string> m_architectures;
    PoolOptions& m_options;
    UnitOfWorkBaseFactory& m_uow_factory;
};

} // namespace bxt::Persistence::Box
