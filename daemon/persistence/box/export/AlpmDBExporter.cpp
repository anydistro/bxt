/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "AlpmDBExporter.h"

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/enums/PoolLocation.h"
#include "persistence/box/record/PackageRecord.h"
#include "persistence/box/store/PackageStoreBase.h"
#include "utilities/alpmdb/Database.h"
#include "utilities/libarchive/Writer.h"
#include "utilities/lmdb/Database.h"
#include "utilities/log/Logging.h"

#include <archive.h>
#include <bits/ranges_algo.h>
#include <coro/sync_wait.hpp>
#include <filesystem>
#include <iterator>
#include <memory>
#include <mutex>
#include <parallel_hashmap/phmap.h>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>

namespace bxt::Persistence::Box {

AlpmDBExporter::AlpmDBExporter(
    PackageStoreBase& package_store,
    ReadOnlyRepositoryBase<Section>& section_repository,
    std::filesystem::path box_path)
    : m_box_path(std::move(box_path)), m_package_store(package_store) {
    auto sections_result = coro::sync_wait(section_repository.all_async());

    if (!sections_result.has_value()) {
        loge("Pool: Can't get available sections, the reason is \"{}\". "
             "Exiting.",
             sections_result.error().what());
        exit(1);
    }

    std::ranges::transform(*sections_result,
                           std::inserter(m_sections, m_sections.end()),
                           SectionDTOMapper::to_dto);

    for (const auto& section : m_sections) {
        std::filesystem::create_directories(m_box_path / std::string(section));
    }
}

coro::task<void> AlpmDBExporter::export_to_disk() {
    phmap::parallel_flat_hash_map<PackageSectionDTO, Archive::Writer> writers;

    for (const auto& section : m_dirty_sections) {
        logd("Exporter: \"{}\" export into the package manager format started",
             std::string(section));
        writers.emplace(section, Archive::Writer());

        archive_write_add_filter_zstd(writers.at(section));
        archive_write_set_format_pax_restricted(writers.at(section));

        writers.at(section).open_filename(
            m_box_path / std::string(section)
            / fmt::format("{}.db.tar.zst", section.repository));

        co_await m_package_store.accept(
            [this, writer = &writers.at(section)](
                std::string_view key,
                const PackageRecord& package) -> Utilities::NavigationAction {
                const auto deserialized = PackageRecord::Id::from_string(key);

                if (!deserialized.has_value()) {
                    loge("Exporter: Key \"{}\" is not valid. Skipping...", key);
                    return Utilities::NavigationAction::Next;
                }

                const auto& [section, name] = *deserialized;

                const auto version =
                    package.descriptions
                        .at(*Core::Domain::select_preferred_pool_location(
                            package.descriptions))
                        .descfile.get("VERSION");

                if (!version.has_value()) {
                    loge("Exporter: Version for \"{}\" is not valid. "
                         "Skipping...",
                         key);

                    return Utilities::NavigationAction::Next;
                }

                logd(
                    "Exporter: Description for \"{}/{}-{}\" is being added... ",
                    std::string(section), name, *version);

                Utilities::AlpmDb::DatabaseUtils::write_buffer_to_archive(
                    *writer, fmt::format("{}-{}/desc", name, *version),
                    package.descriptions
                        .at(*Core::Domain::select_preferred_pool_location(
                            package.descriptions))
                        .descfile.desc);

                logd("Exporter: Description for \"{}/{}-{}\" is added",
                     std::string(section), name, *version);

                return Utilities::NavigationAction::Next;
            },
            std::string(section));

        logd("Exporter: \"{}\" export finished", std::string(section));
    }

    m_dirty_sections.clear();

    co_return;
}

void AlpmDBExporter::add_dirty_sections(
    std::set<Core::Application::PackageSectionDTO>&& sections) {
    m_dirty_sections.insert(std::make_move_iterator(sections.begin()),
                            std::make_move_iterator(sections.end()));
}

} // namespace bxt::Persistence::Box