/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/entities/Package.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
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
                  PackageStoreBase &package_store,
                  WritebackScheduler &writeback_sceduler,
                  ExporterBase &exporter);

    virtual coro::task<TResult> find_by_id_async(TId id) override;
    virtual coro::task<TResult>
        find_first_async(std::function<bool(const Package &)>) override;
    virtual coro::task<TResults>
        find_async(std::function<bool(const Package &)> condition) override;
    virtual coro::task<TResults> all_async() override;

    virtual coro::task<WriteResult<void>>
        add_async(const Package entity) override;
    virtual coro::task<WriteResult<void>>
        add_async(const std::vector<Package> entity) override;

    virtual coro::task<WriteResult<void>>
        update_async(const Package entity) override;
    virtual coro::task<WriteResult<void>> remove_async(const TId id) override;

    virtual coro::task<TResults>
        find_by_section_async(const Section section) override;

    virtual coro::task<TResults> find_by_section_async(
        const Section section,
        const std::function<bool(const Package &)> predicate) override;

    virtual coro::task<TResult> find_by_section_async(const Section section,
                                                      const Name name) override;

    virtual coro::task<UnitOfWorkBase::Result<void>> commit_async() override;
    virtual coro::task<UnitOfWorkBase::Result<void>> rollback_async() override;

private:
    BoxOptions m_options;

    PackageStoreBase &m_package_store;

    ExporterBase &m_exporter;
    WritebackScheduler &m_scheduler;

    std::filesystem::path m_root_path;

    std::vector<Package> m_to_add;
    std::vector<TId> m_to_remove;
    std::vector<Package> m_to_update;
};

} // namespace bxt::Persistence::Box
