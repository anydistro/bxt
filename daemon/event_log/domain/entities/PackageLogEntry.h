/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/entities/Section.h"
#include "core/domain/enums/PoolLocation.h"
#include "core/domain/value_objects/PackageVersion.h"
#include "event_log/domain/enums/LogEntryType.h"

#include <optional>
#include <string>
#include <utility>

namespace bxt::EventLog::Domain {
class PackageLogEntry {
public:
    PackageLogEntry(
        LogEntryType type,
        Core::Domain::Section section,
        std::string name,
        Core::Domain::PoolLocation location,
        std::optional<Core::Domain::PackageVersion> version = std::nullopt)
        : m_type(type),
          m_section(std::move(section)),
          m_name(std::move(name)),
          m_location(location),
          m_version(std::move(version)) {}

    LogEntryType type() const { return m_type; }
    Core::Domain::Section section() const { return m_section; }
    const std::string& name() const { return m_name; }
    Core::Domain::PoolLocation location() const { return m_location; }
    const std::optional<Core::Domain::PackageVersion>& version() const {
        return m_version;
    }

private:
    LogEntryType m_type;
    Core::Domain::Section m_section;
    std::string m_name;
    Core::Domain::PoolLocation m_location;
    std::optional<Core::Domain::PackageVersion> m_version = std::nullopt;
};

} // namespace bxt::EventLog::Domain
