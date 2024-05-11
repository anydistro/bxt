/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "presentation/messages/SectionMessages.h"

#include <string>
#include <unordered_map>

namespace bxt::Presentation {

struct SnapRequest {
    SectionRequest source;
    SectionRequest target;
};

struct PoolEntryResponse {
    std::string version;
    bool has_signature;
};

struct PackageResponse {
    std::string name;
    Core::Application::PackageSectionDTO section;
    std::unordered_map<std::string, PoolEntryResponse> pool_entries;
    std::string preferred_location;
};
} // namespace bxt::Presentation
