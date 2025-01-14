/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "presentation/messages/SectionMessages.h"

#include <vector>

namespace bxt::Presentation {

struct ExportDatabaseRequest {
    std::vector<SectionRequest> sections;
};

} // namespace bxt::Presentation
