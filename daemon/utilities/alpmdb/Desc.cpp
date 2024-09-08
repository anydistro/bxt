/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "Desc.h"

#include "utilities/alpmdb/DescFormatter.h"
#include "utilities/alpmdb/PkgInfo.h"
#include "utilities/libarchive/Error.h"
#include "utilities/libarchive/Reader.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <expected>
#include <fmt/format.h>
#include <frozen/set.h>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <optional>

namespace bxt::Utilities::AlpmDb {
std::optional<std::string> Desc::get(std::string const& key) const {
    auto prepared_key = fmt::format("%{}%\n", key);
    auto value_begin = desc.find(prepared_key);

    if (value_begin == std::string::npos) {
        return {};
    }

    value_begin += prepared_key.size();
    auto value_end = desc.find("\n", value_begin);

    if (value_end == std::string::npos) {
        return {};
    }

    return desc.substr(value_begin, value_end - value_begin);
}

Desc::Result<Desc> Desc::parse_package(std::filesystem::path const& filepath,
                                       std::string const& signature,
                                       bool create_files) {
    std::ostringstream desc;
    std::ostringstream files;

    Archive::Reader file_reader;

    archive_read_support_filter_all(file_reader);
    archive_read_support_format_all(file_reader);

    auto const package_infos = file_reader.open_filename(filepath);

    if (!package_infos.has_value()) {
        return std::unexpected(
            ParseError(ParseError::ErrorType::InvalidArchive, std::move(package_infos.error())));
    }

    PkgInfo package_info;

    bool found = false;
    for (auto& [header, entry] : file_reader) {
        if (!header) {
            continue;
        }
        std::string pathname = archive_entry_pathname(*header);

        if (!pathname.ends_with(".PKGINFO") && !pathname.starts_with("/.")) {
            if (create_files) {
                files << archive_entry_pathname(*header) << "\n";
            }
            continue;
        }
        found = true;

        auto contents = entry.read_all();

        if (!contents.has_value()) {
            if (auto const invalidentry =
                    std::get_if<Archive::InvalidEntryError>(&contents.error())) {
                return std::unexpected(
                    ParseError(ParseError::ErrorType::InvalidArchive, std::move(*invalidentry)));
            } else {
                return std::unexpected(ParseError(
                    ParseError::ErrorType::InvalidArchive,
                    std::move(*std::get_if<Archive::LibArchiveError>(&contents.error()))));
            }
        }

        package_info.parse(
            std::string_view {reinterpret_cast<char*>(contents->data()), contents->size()});

        if (!create_files) {
            break;
        }
    }

    if (!found) {
        return std::unexpected(ParseError(ParseError::ErrorType::NoPackageInfo));
    }

    DescFormatter formatter {package_info, filepath, signature};

    desc << formatter.format();

    return Desc {.desc = desc.str(), .files = files.str()};
}

} // namespace bxt::Utilities::AlpmDb
