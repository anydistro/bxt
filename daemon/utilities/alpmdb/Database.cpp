/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "Database.h"

#include "nonstd/expected.hpp"
#include "parallel_hashmap/phmap.h"
#include "utilities/Error.h"
#include "utilities/alpmdb/Desc.h"
#include "utilities/errors/DatabaseError.h"
#include "utilities/libarchive/Error.h"
#include "utilities/log/Logging.h"

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <coro/latch.hpp>
#include <execution>
#include <filesystem>
#include <fmt/format.h>
#include <iterator>
#include <utilities/libarchive/Reader.h>
#include <utilities/libarchive/Writer.h>

namespace bxt::Utilities::AlpmDb::DatabaseUtils {

constexpr static frozen::set<frozen::string, 3> supported_package_extensions = {
    "pkg.tar.gz", "pkg.tar.xz", "pkg.tar.zst"};

coro::task<Result<phmap::parallel_flat_hash_map<std::string, Desc>>>
    parse_packages(const std::set<std::string> packages) {
    phmap::parallel_flat_hash_map<std::string, Desc> descriptions;

    coro::latch latch {static_cast<ptrdiff_t>(packages.size())};

    accept(packages, [&descriptions, &latch](const std::string& name,
                                             const Desc& description) {
        descriptions.lazy_emplace_l(
            name,
            [description, &latch, name](auto& value) {
                value.second = description;
                latch.count_down();
            },
            [description, &latch, name](const auto& ctor) {
                ctor(name, description);
                latch.count_down();
            });
    });

    co_await latch;

    co_return descriptions;
}

void create_symlinks(std::filesystem::path path) {
    std::string name = path.stem();

    std::error_code ec;
    std::filesystem::create_symlink(
        path.parent_path() / fmt::format("{}.db.tar.zst", name),
        path.parent_path() / fmt::format("{}.db", name), ec);

    std::filesystem::create_symlink(
        path.parent_path() / fmt::format("{}.files.tar.zst", name),
        path.parent_path() / fmt::format("{}.files", name), ec);
}

coro::task<Result<void>>
    save(phmap::parallel_flat_hash_map<std::string, Desc> descriptions,
         std::filesystem::path path) {
    Archive::Writer db_writer, files_writer;

    logd("Saving entries to {}.db.tar.zst...\n", path.string());

    archive_write_add_filter_zstd(db_writer);
    archive_write_set_format_pax_restricted(db_writer);

    db_writer.open_filename(path);

    archive_write_add_filter_zstd(files_writer);
    archive_write_set_format_pax_restricted(files_writer);
    files_writer.open_filename(path);

    for (const auto& [name, description] : descriptions) {
        logd("Description for {} is being added...", name);

        write_buffer_to_archive(db_writer, fmt::format("{}/desc", name),
                                description.desc);

        write_buffer_to_archive(files_writer, fmt::format("{}/files", name),
                                description.files);
    }

    create_symlinks(path);

    co_return {};
}

coro::task<Result<phmap::parallel_flat_hash_map<std::string, Desc>>>
    load(std::filesystem::path path) {
    phmap::parallel_flat_hash_map<std::string, Desc> descriptions;
    Archive::Reader db_reader;

    archive_read_support_format_all(db_reader);
    archive_read_support_filter_all(db_reader);

    auto opened = db_reader.open_filename(path);

    if (!opened.has_value()) {
        co_return bxt::make_error_with_source<DatabaseError>(
            std::move(opened.error()), DatabaseError::ErrorType::IOError);
    }

    for (auto& [header, data] : db_reader) {
        std::filesystem::path path = archive_entry_pathname(*header);

        std::vector<std::string> parts;

        for (const auto& part : path) {
            parts.emplace_back(part);
        }

        if (parts.size() != 2) continue;

        if (parts[1] != "desc") continue;

        auto buffer = data.read_all();
        if (!buffer.has_value()) {
            if (const auto& buffer_err =
                    std::get_if<Archive::InvalidEntryError>(&buffer.error())) {
                co_return bxt::make_error_with_source<DatabaseError>(
                    std::move(*buffer_err),
                    DatabaseError::ErrorType::DatabaseMalformedError);
            } else {
                co_return bxt::make_error_with_source<DatabaseError>(
                    std::move(*std::get_if<Archive::LibArchiveError>(
                        &buffer.error())),
                    DatabaseError::ErrorType::DatabaseMalformedError);
            }
        }

        Desc desc({buffer->begin(), buffer->end()});

        descriptions[parts[0]] = desc;
    }

    co_return {};
}

coro::task<Result<void>>
    accept(std::set<std::string> files,
           std::function<void(const std::string& name, const Desc& description)>
               visitor) {
    for (const auto& package : files) {
        auto description = Desc::parse_package(package);
        if (!description.has_value()) {
            co_return bxt::make_error<DatabaseError>(
                DatabaseError::ErrorType::InvalidEntityError);
        }
        const auto name = description->get("NAME");

        const auto version = description->get("VERSION");

        if (!name.has_value() || !version.has_value()) { continue; }

        visitor(fmt::format("{}-{}", *name, *version), *description);
    }
}

} // namespace bxt::Utilities::AlpmDb::DatabaseUtils
