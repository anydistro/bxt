/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "event_log/domain/entities/PackageLogEntry.h"

#include <utility>

namespace bxt::EventLog::Domain {
class PackageUpdateLogEntry {
public:
    PackageUpdateLogEntry(PackageLogEntry package_log_entry,
                          PackageLogEntry previous_package_log_entry)
        : package_log_entry(std::move(package_log_entry))
        , previous_package_log_entry(std::move(previous_package_log_entry)) {
    }

    PackageLogEntry const& package() const {
        return package_log_entry;
    }
    PackageLogEntry const& previous_package() const {
        return previous_package_log_entry;
    }

private:
    PackageLogEntry package_log_entry;
    PackageLogEntry previous_package_log_entry;
};

} // namespace bxt::EventLog::Domain
