/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#include "PackagePoolEntry.h"
namespace bxt::Core::Domain {

PackagePoolEntry::Result<PackagePoolEntry> PackagePoolEntry::parse_file_path(
    const std::filesystem::path& file_path,
    const std::optional<std::filesystem::path>& signature_path) {
    const std::string filename = file_path.filename();

    std::vector<std::string> substrings;

    boost::split(substrings, filename, boost::is_any_of("-"));

    auto subsize = substrings.size();

    if (subsize < 4) {
        return bxt::make_error<ParsingError>(
            ParsingError::ErrorCode::InvalidFilename);
    }

    auto release_substr = substrings[subsize - 2];
    auto version_substr = substrings[subsize - 3];

    auto version_pos = filename.find(version_substr);

    auto version = PackageVersion::from_string(
        fmt::format("{}-{}", version_substr, release_substr));

    if (!version.has_value()) {
        return bxt::make_error_with_source<ParsingError>(
            std::move(version.error()),
            ParsingError::ErrorCode::InvalidVersion);
    }

    PackagePoolEntry result(file_path, {}, *version);

    if (signature_path.has_value()) {
        result.m_signature_path = signature_path;
        return result;
    }

    const auto deduced_signature_path =
        fmt::format("{}.sig", file_path.string());

    if (std::filesystem::exists(deduced_signature_path)) {
        result.m_signature_path = deduced_signature_path;
    }

    return result;
}
} // namespace bxt::Core::Domain
