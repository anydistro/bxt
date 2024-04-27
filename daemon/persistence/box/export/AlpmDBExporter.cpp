/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
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
#include <coro/sync_wait.hpp>
#include <filesystem>
#include <iterator>
#include <memory>
#include <mutex>
#include <parallel_hashmap/phmap.h>
#include <string_view>
#include <system_error>
#include <thread>
#include <unordered_map>
#include <utility>

namespace bxt::Persistence::Box {

void create_relative_symlink(const std::filesystem::path& target,
                             const std::filesystem::path& link) {
    if (std::filesystem::is_symlink(link)) { return; }
    std::error_code ec;
    const auto relative_target =
        std::filesystem::relative(target, link.parent_path(), ec);

    if (ec) {
        logf("Failed to get relative symlink path. The error is \"{}\". Box "
             "will be malformed, exiting.",
             ec.message());
        exit(1);
    }

    std::filesystem::create_symlink(relative_target, link, ec);
    if (ec) {
        logf("Failed to create symlink. The error is \"{}\". Box will be "
             "malformed, exiting.",
             ec.message());
        exit(1);
    }
}

AlpmDBExporter::AlpmDBExporter(
    BoxOptions& box_options,
    PackageStoreBase& package_store,
    ReadOnlyRepositoryBase<Section>& section_repository)
    : m_box_path(box_options.box_path), m_package_store(package_store) {
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

                const auto preferred_description = package.descriptions.at(
                    *Core::Domain::select_preferred_pool_location(
                        package.descriptions));

                Utilities::AlpmDb::DatabaseUtils::write_buffer_to_archive(
                    *writer, fmt::format("{}-{}/desc", name, *version),
                    preferred_description.descfile.desc);

                const auto filepath_link = std::filesystem::absolute(
                    m_box_path / std::string(section)
                    / preferred_description.filepath.filename());

                create_relative_symlink(preferred_description.filepath,
                                        filepath_link);

                if (preferred_description.signature_path) {
                    const auto signature_link = std::filesystem::absolute(
                        m_box_path / std::string(section)
                        / preferred_description.signature_path->filename());

                    create_relative_symlink(
                        *preferred_description.signature_path, signature_link);
                }

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
