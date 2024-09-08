/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/domain/enums/PoolLocation.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "persistence/box/BoxOptions.h"
#include "persistence/box/pool/PoolBase.h"
#include "PoolOptions.h"

#include <filesystem>
#include <parallel_hashmap/phmap.h>
#include <string>

namespace bxt::Persistence::Box {
class PackageRecord;

class Pool : public PoolBase {
public:
    Pool(BoxOptions& box_options,
         PoolOptions& options,
         ReadOnlyRepositoryBase<Section>& section_repository,
         UnitOfWorkBaseFactory& uow_factory);

    PoolBase::Result<PackageRecord> move_to(PackageRecord const& package) override;

    PoolBase::Result<void> remove(PackageRecord const& package) override;

    PoolBase::Result<PackageRecord> path_for_package(PackageRecord const& package) const override;

    void count_links(PackageRepositoryBase& package_repository);

private:
    std::string format_target_path(Core::Domain::PoolLocation location,
                                   std::string const& arch,
                                   std::optional<std::string> const& filename = {}) const;

    std::filesystem::path m_pool_path;
    std::set<std::string> m_architectures;
    PoolOptions& m_options;
    UnitOfWorkBaseFactory& m_uow_factory;

    phmap::parallel_flat_hash_map<std::filesystem::path, size_t> m_pool_package_link_counts;
};

} // namespace bxt::Persistence::Box
