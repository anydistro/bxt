/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/entities/Section.h"
#include "core/domain/repositories/ReadOnlyRepositoryBase.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "persistence/box/export/AlpmDBExporter.h"
#include "persistence/box/pool/PoolBase.h"
#include "persistence/box/record/PackageRecord.h"
#include "persistence/box/store/PackageStoreBase.h"
#include "persistence/box/writeback/WritebackScheduler.h"
#include "utilities/NavigationAction.h"
#include "utilities/lmdb/Database.h"
#include "utilities/lmdb/Environment.h"
#include "utilities/locked.h"

#include <kangaru/service.hpp>
#include <memory>
#include <vector>

namespace bxt::Persistence::Box {

class LMDBPackageStore : public PackageStoreBase {
public:
    LMDBPackageStore(BoxOptions& box_options,
                     std::shared_ptr<Utilities::LMDB::Environment> env,
                     PoolBase& pool,
                     ReadOnlyRepositoryBase<Section>& section_repository,
                     const std::string_view name);

    ~LMDBPackageStore() override = default;

    coro::task<std::expected<void, DatabaseError>>
        add(const PackageRecord package,
            std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<std::expected<void, DatabaseError>>
        delete_by_id(const PackageRecord::Id package_id,
                     std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<std::expected<void, DatabaseError>>
        update(const PackageRecord package,
               std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<std::expected<std::vector<PackageRecord>, DatabaseError>>
        find_by_section(PackageSectionDTO section,
                        std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<std::expected<void, DatabaseError>>
        accept(std::function<Utilities::NavigationAction(
                   std::string_view key, const PackageRecord& value)> visitor,
               std::string_view prefix,
               std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<std::expected<void, DatabaseError>>
        accept(std::function<Utilities::NavigationAction(
                   std::string_view key, const PackageRecord& value)> visitor,
               std::shared_ptr<UnitOfWorkBase> uow) override;

private:
    std::filesystem::path m_root_path;
    PoolBase& m_pool;
    Utilities::LMDB::Database<PackageRecord> m_db;
    ReadOnlyRepositoryBase<Section>& m_section_repository;
};

} // namespace bxt::Persistence::Box
