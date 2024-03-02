/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "Package.h"
#include "core/domain/enums/LogEntryType.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>

namespace bxt::Core::Domain {

class PackageLogEntry {
public:
    PackageLogEntry(
        Package package,
        LogEntryType type,
        const boost::uuids::uuid& uuid = boost::uuids::random_generator()(),
        std::chrono::time_point<std::chrono::system_clock> time =
            std::chrono::system_clock::now())
        : m_id(uuid), m_time(time), m_type(type), m_package(package) {}

    std::string id() const { return boost::uuids::to_string(m_id); }

    std::chrono::time_point<std::chrono::system_clock> time() const {
        return m_time;
    }
    void set_time(std::chrono::time_point<std::chrono::system_clock> new_time) {
        m_time = new_time;
    }

    LogEntryType type() const { return m_type; }
    void set_type(LogEntryType new_type) { m_type = new_type; }

    Package package() const { return m_package; }
    void set_package(const Package& new_package) { m_package = new_package; }

private:
    boost::uuids::uuid m_id;
    std::chrono::time_point<std::chrono::system_clock> m_time;
    LogEntryType m_type;
    Package m_package;
};

} // namespace bxt::Core::Domain
