/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/enums/LogEntryType.h"
#include "presentation/messages/PackageMessages.h"

#include <string>

namespace bxt::Presentation {

struct LogEntryReponse {
    std::string id;
    uint64_t time;
    Core::Domain::LogEntryType type;
    PackageResponse package;
};

using LogResponse = std::vector<LogEntryReponse>;
} // namespace bxt::Presentation
