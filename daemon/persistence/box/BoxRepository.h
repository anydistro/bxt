/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/entities/Package.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "coro/task.hpp"
#include "persistence/box/BoxOptions.h"
#include "persistence/box/export/ExporterBase.h"
#include "persistence/box/store/PackageStoreBase.h"
#include "persistence/box/writeback/WritebackScheduler.h"
#include "utilities/alpmdb/Database.h"

#include <coro/io_scheduler.hpp>
#include <coro/sync_wait.hpp>
#include <functional>
#include <memory>

namespace bxt::Persistence::Box {

class BoxRepository : public Core::Domain::PackageRepositoryBase {
public:
    BoxRepository(BoxOptions options,
                  PackageStoreBase& package_store,
                  WritebackScheduler& writeback_sceduler,
                  ExporterBase& exporter);

    coro::task<TResult> find_by_id_async(TId id, std::shared_ptr<UnitOfWorkBase> uow) override;
    coro::task<TResult> find_first_async(std::function<bool(Package const&)>,
                                         std::shared_ptr<UnitOfWorkBase> uow) override;
    coro::task<TResults> find_async(std::function<bool(Package const&)> condition,
                                    std::shared_ptr<UnitOfWorkBase> uow) override;
    coro::task<TResults> all_async(std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<WriteResult<void>> add_async(Package const entity,
                                            std::shared_ptr<UnitOfWorkBase> uow) override;
    coro::task<WriteResult<void>> add_async(std::vector<Package> const entity,
                                            std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<WriteResult<void>> update_async(Package const entity,
                                               std::shared_ptr<UnitOfWorkBase> uow) override;
    coro::task<WriteResult<void>> update_async(std::vector<Package> const entity,
                                               std::shared_ptr<UnitOfWorkBase> uow) override;
    coro::task<WriteResult<void>> delete_async(TId const id,
                                               std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<WriteResult<void>> delete_async(std::vector<TId> const ids,
                                               std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<TResults> find_by_section_async(Section const section,
                                               std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<TResults> find_by_section_async(Section const section,
                                               std::function<bool(Package const&)> const predicate,
                                               std::shared_ptr<UnitOfWorkBase> uow) override;

    coro::task<TResult> find_by_section_async(Section const section,
                                              Name const name,
                                              std::shared_ptr<UnitOfWorkBase> uow) override;

private:
    void make_writeback_hook(Section const section, std::shared_ptr<UnitOfWorkBase> uow);
    BoxOptions m_options;

    PackageStoreBase& m_package_store;

    ExporterBase& m_exporter;
    WritebackScheduler& m_scheduler;

    std::filesystem::path m_root_path;
};

} // namespace bxt::Persistence::Box
