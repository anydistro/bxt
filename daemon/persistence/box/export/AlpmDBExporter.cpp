/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "AlpmDBExporter.h"

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/enums/PoolLocation.h"
#include "utilities/Error.h"
#include "utilities/NavigationAction.h"
#include "utilities/alpmdb/Database.h"
#include "utilities/errors/FsError.h"
#include "utilities/libarchive/Error.h"

#include <archive.h>
#include <coro/sync_wait.hpp>
#include <expected>
#include <filesystem>
#include <iterator>
#include <memory>
#include <mutex>
#include <parallel_hashmap/phmap.h>
#include <string_view>
#include <system_error>

namespace bxt::Persistence::Box {
// Creates the symlink relative to target
std::expected<void, FsError>
    create_relative_symlink(const std::filesystem::path& target,
                            const std::filesystem::path& link) {
    std::error_code ec;

    if (std::filesystem::exists(link)) {
        ec.assign(static_cast<int>(std::errc::file_exists),
                  std::generic_category());
        return bxt::make_error<FsError>(ec);
    }

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

        /// TODO: We need to make this way more robust. At least a full backup
        /// would work but maybe we can do something more smart...
        if (!cleanup_section(section)) { co_return; }

        auto writer = setup_alpmdb_writer(section);

        if (!writer.has_value()) {
            logf("Exporter: Writer cannot be created, the error is \"{}\". "
                 "Stopping...",
                 writer.error().what());
            co_return;
        }

        writers.emplace(section, std::move(*writer));

        co_await m_package_store.accept(
            [this, writer = &writers.at(section)](
                std::string_view key, const PackageRecord& package) {
                if (auto export_ok = export_package(*writer, key, package);
                    !export_ok) {
                    logf(fmt::format("Exporter: {}. Stopping...",
                                     export_ok.error()));
                    return Utilities::NavigationAction::Stop;
                }

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
// Factory function for ALPM .db archive writer
std::expected<Archive::Writer, bxt::Error>
    AlpmDBExporter::setup_alpmdb_writer(const PackageSectionDTO& section) {
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
// Cleans up section before the export by removing all it's content
std::expected<void, FsError>
    AlpmDBExporter::cleanup_section(const PackageSectionDTO& section) {
    std::error_code ec;

    std::filesystem::directory_iterator directory_iterator;

    constexpr auto handle_error = [](const auto& ec) {
        logf("Exporter: Can't wipe the directory before the export, the "
             "error is "
             "\"{}\". Stopping...",
             ec.message());
        return bxt::make_error<FsError>(ec);
    };

    if (directory_iterator = std::filesystem::directory_iterator(
            m_box_path / std::string(section), ec);
        ec) {
        return handle_error(ec);
    }

    for (const auto& entry : directory_iterator) {
        if (!std::filesystem::remove_all(entry.path(), ec)) {
            return handle_error(ec);
        }
    }

    return {};
}

struct PackageDetails {
    PackageSectionDTO section;
    std::string name;
    std::string version;
    PoolLocation preferred_location;
};
// Validates and extracts essential package details from both key and record
std::expected<PackageDetails, std::string>
    validate_package_key(std::string_view key, const PackageRecord& package) {
    auto deserialized = PackageRecord::Id::from_string(key);
    if (!deserialized.has_value()) {
        return std::unexpected(fmt::format("Invalid package key: '{}'.", key));
    }

    const auto& [section, name] = *deserialized;

    auto location = select_preferred_pool_location(package.descriptions);

    if (!location.has_value()) {
        return std::unexpected(
            fmt::format("Can't select preferred location for '{}'", key));
    }

    std::optional<std::string> version_string;

    auto description_it = package.descriptions.find(*location);
    if (description_it == package.descriptions.end()) {
        return std::unexpected(fmt::format(
            "Package details not found for preferred location: '{}'.", key));
    }
    if (!(version_string = description_it->second.descfile.get("VERSION"))
             .has_value()
        || version_string->empty()) {
        return std::unexpected(
            fmt::format("No valid version for package '{}'.", key));
    }
    return PackageDetails {section, name, *version_string, *location};
}

// Writes the contents of package description file to section's ALPM Database
// archive
std::expected<void, std::string>
    write_package_description_to_alpmdb(Archive::Writer& alpmdb_writer,
                                        const PackageDetails& details,
                                        const PackageRecord& package) {
    const auto& [section, name, version, preferred_location] = details;
    auto desc = package.descriptions.at(preferred_location).descfile.desc;
    auto desc_path = fmt::format("{}-{}/desc", name, version);

    if (auto write_ok =
            Utilities::AlpmDb::DatabaseUtils::write_buffer_to_archive(
                alpmdb_writer, desc_path, desc);
        !write_ok) {
        return std::unexpected(
            fmt::format("Failed to write description for '{}/{}-{}'.",
                        std::string(section), name, version));
    }
    return {};
}

// Symlinks (usually pool) package and optionally it's signature to the
// specified section
std::expected<void, std::string>
    symlink_package_files(const PackageDetails& details,
                          const PackageRecord& package,
                          const std::filesystem::path& box_path) {
    auto [section, name, version, preferred_location] = details;

    auto description = package.descriptions.at(preferred_location);

    auto link_file_path = std::filesystem::absolute(
        box_path / std::string(section) / description.filepath.filename());

    if (auto link_ok =
            create_relative_symlink(description.filepath, link_file_path);
        !link_ok) {
        return std::unexpected(
            fmt::format("Failed to link package file for '{}/{}-{}'.",
                        std::string(section), name, version));
    }

    if (description.signature_path.has_value()) {
        auto link_file_path =
            std::filesystem::absolute(box_path / std::string(section)
                                      / description.signature_path->filename());

        if (auto link_ok = create_relative_symlink(*description.signature_path,
                                                   link_file_path);
            !link_ok) {
            return std::unexpected(
                fmt::format("Failed to link signature file for '{}/{}-{}'.",
                            std::string(section), name, version));
        }
    }
    return {};
}
// Exports package into the ALPM repository format by writing it's description
// into the .db file and symlinking package and signature files for specified
// section
std::expected<void, std::string>
    AlpmDBExporter::export_package(Archive::Writer& writer,
                                   std::string_view key,
                                   const PackageRecord& package) {
    auto validated_details = validate_package_key(key, package);
    if (!validated_details.has_value()) {
        return std::unexpected(validated_details.error());
    }

    auto write_result = write_package_description_to_alpmdb(
        writer, *validated_details, package);
    if (!write_result.has_value()) {
        return std::unexpected(write_result.error());
    }

    auto file_management_result =
        symlink_package_files(*validated_details, package, m_box_path);
    if (!file_management_result.has_value()) {
        return std::unexpected(file_management_result.error());
    }

    return {};
}

} // namespace bxt::Persistence::Box
