/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "Package.h"

#include "core/domain/value_objects/PackagePoolEntry.h"
#include "core/domain/value_objects/PackageVersion.h"
#include "fmt/core.h"
#include "scn/tuple_return/tuple_return.h"
#include "utilities/Error.h"
#include "utilities/alpmdb/Desc.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <cctype>
#include <filesystem>
#include <fmt/format.h>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

bool check_valid_name(std::string_view name) {
    if (name.empty()) { return false; }

    static constexpr std::string_view valid = "@._+-";

    return name[0] != '-' && name[0] != '.';

    /// TODO: This code should be uncommented when we will be sure every our
    /// package follows the
    /// https://wiki.archlinux.org/title/Arch_package_guidelines#Package_naming
    /// convention
    //    && std::ranges::all_of(name, [](const char& ch) {
    //           return ((std::isalpha(ch)) && std::islower(ch))
    //                  || std::isdigit(ch)
    //                  || (valid.find(ch) != std::string::npos);
    //       });
}

namespace bxt::Core::Domain {

std::optional<std::string>
    Package::parse_file_name(const std::string& filename) {
    std::vector<std::string> substrings;
    boost::split(substrings, filename, boost::is_any_of("-"));

    if (substrings.size() < 4) { return {}; }

    auto version_pos = filename.find(substrings[substrings.size() - 3]);
    std::string name = filename.substr(0, version_pos - 1);

    if (!check_valid_name(name)) { return {}; }

    return name;
}
Package::Result<Package> Package::from_file_path(
    const Section& section,
    const PoolLocation location,
    const std::filesystem::path& filepath,
    const std::optional<std::filesystem::path>& signature_path) {
    auto pool_entry =
        PackagePoolEntry::parse_file_path(filepath, signature_path);

    if (!pool_entry.has_value()) {
        return bxt::make_error_with_source<ParseError>(
            std::move(pool_entry.error()));
    }

    auto name = parse_file_name(filepath.filename());

    if (!name.has_value()) { return bxt::make_error<ParseError>(); }

    Package result(section, *name, false);
    result.pool_entries().emplace(location, *pool_entry);
    return result;
}

} // namespace bxt::Core::Domain
