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

std::expected<void, FsError>
    create_relative_symlink(const std::filesystem::path& target,
                            const std::filesystem::path& link) {
    if (std::filesystem::is_symlink(link)) { return {}; }
    std::error_code ec;

    const auto relative_target =
        std::filesystem::relative(target, link.parent_path(), ec);
    if (ec) { return bxt::make_error<FsError>(ec); }

    std::filesystem::create_symlink(relative_target, link, ec);
    if (ec) { return bxt::make_error<FsError>(ec); }

    return {};
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
        logi("Exporter: \"{}\" export into the package manager format started",
             std::string(section));

        auto writer = setup_writer(section);

        if (!writer.has_value()) {
            logf("Exporter: Writer cannot be created, the error is \"{}\". "
                 "Stopping...",
                 writer.error().what());
            co_return;
        }

        writers.emplace(section, std::move(*writer));

        co_await m_package_store.accept(
            [this, writer = &writers.at(section)](
                std::string_view key,
                const PackageRecord& package) -> Utilities::NavigationAction {
                const auto deserialized = PackageRecord::Id::from_string(key);

                if (!deserialized.has_value()) {
                    loge("Exporter: Key \"{}\" is not valid. Skipping...", key);
                    return Utilities::NavigationAction::Stop;
                }

                const auto& [section, name] = *deserialized;

                const auto preferred_location =
                    Core::Domain::select_preferred_pool_location(
                        package.descriptions);

                if (!preferred_location.has_value()) {
                    logf("Exporter: Can't find preferred location for \"{}\". "
                         "Stopping...",
                         name);
                    return Utilities::NavigationAction::Stop;
                }

                const auto version =
                    package.descriptions.at(*preferred_location)
                        .descfile.get("VERSION");

                if (!version.has_value()) {
                    logf("Exporter: Version for \"{}\" is not valid. "
                         "Stopping...",
                         key);

                    return Utilities::NavigationAction::Stop;
                }

                logi(
                    "Exporter: Description for \"{}/{}-{}\" is being added... ",
                    std::string(section), name, *version);

                const auto preferred_description = package.descriptions.at(
                    *Core::Domain::select_preferred_pool_location(
                        package.descriptions));

                if (auto write_ok = Utilities::AlpmDb::DatabaseUtils::
                        write_buffer_to_archive(
                            *writer, fmt::format("{}-{}/desc", name, *version),
                            preferred_description.descfile.desc);
                    !write_ok) {
                    logf("Exporter: Can't write \"{}/{}-{}\" description to "
                         "the archive. The error is \"{}\"",
                         std::string(section), name, *version,
                         write_ok.error().what());

                    return Utilities::NavigationAction::Stop;
                }

                const auto filepath_link = std::filesystem::absolute(
                    m_box_path / std::string(section)
                    / preferred_description.filepath.filename());

                if (auto link_created_ok = create_relative_symlink(
                        preferred_description.filepath, filepath_link);
                    !link_created_ok) {
                    logf("Exporter: Can't link the package file for "
                         "\"{}/{}-{}\". The error is \"{}\". Stopping the "
                         "export...",
                         std::string(section), name, *version,
                         link_created_ok.error().what());

                    return Utilities::NavigationAction::Stop;
                }

                if (preferred_description.signature_path) {
                    const auto signature_link = std::filesystem::absolute(
                        m_box_path / std::string(section)
                        / preferred_description.signature_path->filename());

                    if (auto link_created_ok = create_relative_symlink(
                            *preferred_description.signature_path,
                            signature_link);
                        !link_created_ok) {
                        logf("Exporter: Can't link the signature file for "
                             "\"{}/{}-{}\". The error is \"{}\". Stopping the "
                             "export...",
                             std::string(section), name, *version,
                             link_created_ok.error().what());

                        return Utilities::NavigationAction::Stop;
                    }
                }

                logi("Exporter: Description and link for \"{}/{}-{}\" is added",
                     std::string(section), name, *version);

                return Utilities::NavigationAction::Next;
            },
            std::string(section));

        logi("Exporter: \"{}\" export finished", std::string(section));
    }

    m_dirty_sections.clear();

    co_return;
}

void AlpmDBExporter::add_dirty_sections(
    std::set<Core::Application::PackageSectionDTO>&& sections) {
    m_dirty_sections.insert(std::make_move_iterator(sections.begin()),
                            std::make_move_iterator(sections.end()));
}

std::expected<Archive::Writer, bxt::Error>
    AlpmDBExporter::setup_writer(const PackageSectionDTO& section) {
    Archive::Writer writer;

    if (archive_write_add_filter_zstd(writer) < ARCHIVE_WARN) {
        return bxt::make_error<Archive::LibArchiveError>(std::move(writer));
    }
    if (archive_write_set_format_pax_restricted(writer) < ARCHIVE_WARN) {
        return bxt::make_error<Archive::LibArchiveError>(std::move(writer));
    }

    const auto archive_path =
        m_box_path / std::string(section)
        / fmt::format("{}.db.tar.zst", section.repository);

    if (auto open_ok = writer.open_filename(archive_path); !open_ok) {
        return std::unexpected(std::move(open_ok.error()));
    }

    const auto archive_link = m_box_path / std::string(section)
                              / fmt::format("{}.db", section.repository);

    if (auto link_created_ok =
            create_relative_symlink(archive_path, archive_link);
        !link_created_ok) {
        return std::unexpected(link_created_ok.error());
    }

    return writer;
}

} // namespace bxt::Persistence::Box
