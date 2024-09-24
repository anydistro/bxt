/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once
#include "core/domain/entities/Section.h"
#include "core/domain/value_objects/Name.h"
#include "event_log/domain/entities/PackageLogEntry.h"

namespace bxt::tests {
using namespace bxt::Core::Domain;
using namespace bxt::EventLog::Domain;
inline PackageLogEntry create_pkg_entry1() {
    Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
    return PackageLogEntry {LogEntryType::Add, section, "pkg1", PoolLocation::Sync,
                            PackageVersion::from_string("1.0").value()};
}

inline PackageLogEntry create_pkg_entry2() {
    Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
    return PackageLogEntry {LogEntryType::Add, section, "pkg2", PoolLocation::Sync,
                            PackageVersion::from_string("2.0").value()};
}

inline PackageLogEntry create_pkg_entry3() {
    Section section {Name {"stable"}, Name {"core"}, Name {"x86_64"}};
    return PackageLogEntry {LogEntryType::Remove, section, "pkg3", PoolLocation::Sync,
                            PackageVersion::from_string("3.0").value()};
}

inline PackageLogEntry create_pkg_entry4() {
    Section section {Name {"testing"}, Name {"extra"}, Name {"x86_64"}};
    return PackageLogEntry {LogEntryType::Add, section, "pkg4", PoolLocation::Sync,
                            PackageVersion::from_string("4.0").value()};
}

inline PackageLogEntry create_pkg_entry5() {
    Section section {Name {"unstable"}, Name {"community"}, Name {"x86_64"}};
    return PackageLogEntry {LogEntryType::Add, section, "pkg5", PoolLocation::Sync,
                            PackageVersion::from_string("5.0").value()};
}

} // namespace bxt::tests
