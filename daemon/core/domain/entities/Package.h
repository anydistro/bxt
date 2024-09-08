/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/entities/AggregateRoot.h"
#include "core/domain/enums/PoolLocation.h"
#include "core/domain/value_objects/Name.h"
#include "core/domain/value_objects/PackageArchitecture.h"
#include "core/domain/value_objects/PackagePoolEntry.h"
#include "core/domain/value_objects/PackageVersion.h"
#include "parallel_hashmap/phmap.h"
#include "Section.h"
#include "utilities/Error.h"

#include <expected>
#include <filesystem>
#include <fmt/format.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace bxt::Core::Domain {

class Package {
public:
    struct TId {
        Section section;
        Name package_name;
    };

    struct ParseError : public bxt::Error {};
    BXT_DECLARE_RESULT(ParseError)

    TId const id() const {
        return {m_section, m_name};
    }
    std::string const& name() const {
        return m_name;
    }
    PackageVersion const version() const {
        return m_pool_entries.at(select_preferred_pool_location(m_pool_entries).value()).version();
    }
    std::filesystem::path const filepath() const {
        return m_pool_entries.at(select_preferred_pool_location(m_pool_entries).value())
            .file_path();
    }
    PoolLocation location() const {
        return select_preferred_pool_location(m_pool_entries).value();
    }

    Package(Section section, std::string const& name, bool is_any)
        : m_section(std::move(section))
        , m_name(name)
        , m_is_any_arch(is_any) {
    }

    static std::optional<std::string> parse_file_name(std::string const& filename);

    static Result<Package>
        from_file_path(Section const& section,
                       PoolLocation const location,
                       std::filesystem::path const& filepath,
                       std::optional<std::filesystem::path> const& signature_path = {});

    virtual ~Package() = default;

    std::string string() const {
        return fmt::format("{}", name());
    }

    Section section() const {
        return m_section;
    }

    void set_name(std::string const& new_name) {
        m_name = new_name;
    }

    void set_section(Section const& new_section) {
        m_section = new_section;
    }

    bool is_any_arch() const {
        return m_is_any_arch;
    }

    phmap::flat_hash_map<PoolLocation, PackagePoolEntry>& pool_entries() {
        return m_pool_entries;
    }

    phmap::flat_hash_map<PoolLocation, PackagePoolEntry> pool_entries() const {
        return m_pool_entries;
    }

private:
    Section m_section;

    Name m_name;
    bool m_is_any_arch;
    phmap::flat_hash_map<PoolLocation, PackagePoolEntry> m_pool_entries;
};

} // namespace bxt::Core::Domain

namespace bxt {
template<> inline std::string to_string(bxt::Core::Domain::Package::TId const& id) {
    return fmt::format("{}/{}", bxt::to_string(id.section), id.package_name);
}
} // namespace bxt
