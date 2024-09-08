/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/entities/Section.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "parallel_hashmap/phmap.h"
#include "persistence/box/BoxOptions.h"
#include "persistence/box/export/ExporterBase.h"
#include "persistence/box/store/PackageStoreBase.h"
#include "utilities/Error.h"
#include "utilities/errors/FsError.h"
#include "utilities/libarchive/Writer.h"

#include <coro/io_scheduler.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace bxt::Persistence::Box {

class AlpmDBExporter : public ExporterBase {
public:
    AlpmDBExporter(BoxOptions& box_options,
                   PackageStoreBase& package_store,
                   ReadOnlyRepositoryBase<Section>& section_repository,
                   UnitOfWorkBaseFactory& uow_factory);

    coro::task<void> export_to_disk() override;
    void add_dirty_sections(std::set<Core::Application::PackageSectionDTO>&& override) override;

private:
    std::expected<Archive::Writer, bxt::Error>
        setup_alpmdb_writer(PackageSectionDTO const& section);

    std::expected<void, FsError> cleanup_section(PackageSectionDTO const& section);

    std::expected<void, std::string>
        export_package(Archive::Writer& writer, std::string_view key, PackageRecord const& package);

    std::filesystem::path m_box_path;
    std::set<Core::Application::PackageSectionDTO> m_sections;
    PackageStoreBase& m_package_store;
    UnitOfWorkBaseFactory& m_uow_factory;

    phmap::parallel_node_hash_set<PackageSectionDTO> m_dirty_sections;
};

} // namespace bxt::Persistence::Box
