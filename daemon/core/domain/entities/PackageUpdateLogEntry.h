/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/entities/PackageLogEntry.h"
namespace bxt::Core::Domain {

class PackageUpdateLogEntry : public PackageLogEntry {
public:
    PackageUpdateLogEntry(
        const Package& package,
        const Package& previous_package,
        const boost::uuids::uuid& uuid = boost::uuids::random_generator()(),
        std::chrono::time_point<std::chrono::system_clock> time =
            std::chrono::system_clock::now())
        : PackageLogEntry(package, LogEntryType::Update, uuid, time),
          m_previous_package(previous_package) {}

    Package previous_package() const { return m_previous_package; }
    void set_previous_package(const Package& new_previous_package) {
        m_previous_package = new_previous_package;
    }

private:
    Package m_previous_package;
};

} // namespace bxt::Core::Domain
