/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "Desc.h"

#include "utilities/libarchive/Error.h"
#include "utilities/libarchive/Reader.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <expected>
#include <fmt/format.h>
#include <frozen/set.h>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

const std::string md5_from_file(const std::string &path) {
    unsigned char result[MD5_DIGEST_LENGTH];
    boost::iostreams::mapped_file_source src(path);
    MD5((unsigned char *)src.data(), src.size(), result);

    std::ostringstream sout;
    sout << std::hex << std::setfill('0');
    for (auto c : result)
        sout << std::setw(2) << (int)c;

    return sout.str();
}
const std::string sha256_from_file(const std::string &path) {
    unsigned char result[SHA256_DIGEST_LENGTH];
    boost::iostreams::mapped_file_source src(path);
    SHA256((unsigned char *)src.data(), src.size(), result);

    std::ostringstream sout;
    sout << std::hex << std::setfill('0');
    for (auto c : result)
        sout << std::setw(2) << (int)c;

    return sout.str();
}

constexpr static frozen::unordered_map<frozen::string, frozen::string, 14>
    m_desc_to_info_mapping {
        {"NAME", "pkgname"},        {"BASE", "pkgbase"},
        {"VERSION", "pkgver"},      {"DESC", "pkgdesc"},
        {"ISIZE", "size"},          {"URL", "url"},
        {"LICENSE", "license"},     {"ARCH", "arch"},
        {"BUILDDATE", "builddate"}, {"PACKAGER", "packager"},
        {"REPLACES", "replaces"},   {"CONFLICTS", "conflict"},
        {"PROVIDES", "provides"},   {"DEPENDS", "depend"}};

constexpr static frozen::set<frozen::string, 19> desc_keys {
    "FILENAME", "NAME",      "VERSION",   "CSIZE",   "ISIZE",
    "MD5SUM",   "SHA256SUM", "PGPSIG",    "LICENSE", "BUILDDATE",
    "REPLACES", "PROVIDES",  "BASE",      "DESC",    "URL",
    "ARCH",     "PACKAGER",  "CONFLICTS", "DEPENDS",
};

namespace bxt::Utilities::AlpmDb {
std::optional<std::string> Desc::get(const std::string &key) const {
    auto prepared_key = fmt::format("%{}%\n", key);
    auto value_begin = desc.find(prepared_key);

    if (value_begin == std::string::npos) { return {}; }

    value_begin += prepared_key.size();
    auto value_end = desc.find("\n", value_begin);

    if (value_end == std::string::npos) { return {}; }

    return desc.substr(value_begin, value_end - value_begin);
}

Desc::Result<Desc> Desc::parse_package(const std::filesystem::path &filepath,
                                       bool create_files) {
    std::ostringstream desc;
    std::ostringstream files;

    Archive::Reader file_reader;

    archive_read_support_filter_all(file_reader);
    archive_read_support_format_all(file_reader);

    const auto package_infos = file_reader.open_filename(filepath);

    if (!package_infos.has_value()) {
        return std::unexpected(ParseError(ParseError::ErrorType::InvalidArchive,
                                          std::move(package_infos.error())));
    }

    PkgInfo infoObj;

    bool found = false;
    for (auto &[a, b] : file_reader) {
        std::string pathname = archive_entry_pathname(*a);
        if (!pathname.ends_with(".PKGINFO") && !pathname.starts_with("/.")) {
            if (create_files) { files << archive_entry_pathname(*a) << "\n"; }
            continue;
        }
        found = true;

        auto contents = b.read_all();

        if (!contents.has_value()) {
            if (const auto invalidentry =
                    std::get_if<Archive::InvalidEntryError>(
                        &contents.error())) {
                return std::unexpected(
                    ParseError(ParseError::ErrorType::InvalidArchive,
                               std::move(*invalidentry)));
            } else {
                return std::unexpected(
                    ParseError(ParseError::ErrorType::InvalidArchive,
                               std::move(*std::get_if<Archive::LibArchiveError>(
                                   &contents.error()))));
            }
        }

        infoObj.parse(reinterpret_cast<char *>(contents->data()));

        if (!create_files) { break; }
    }

    if (!found) {
        return std::unexpected(
            ParseError(ParseError::ErrorType::NoPackageInfo));
    }

    constexpr static char format_string[] = "%{}%\n{}\n\n";

    desc << fmt::format(format_string, "FILENAME",
                        filepath.filename().string());
    desc << fmt::format(format_string, "CSIZE",
                        std::to_string(std::filesystem::file_size(filepath)));

    desc << fmt::format(format_string, "MD5SUM", md5_from_file(filepath));
    desc << fmt::format(format_string, "SHA256SUM", sha256_from_file(filepath));

    for (const auto &mapping : m_desc_to_info_mapping) {
        auto values = infoObj.values(
            std::string {mapping.second.data(), mapping.second.size()});

        if (values.size() == 0) continue;

        desc << fmt::format(
            format_string,
            std::string {mapping.first.data(), mapping.first.size()},
            boost::join(values, " "));
    }

    return Desc {.desc = desc.str(), .files = files.str()};
}

} // namespace bxt::Utilities::AlpmDb
