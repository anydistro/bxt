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
#include <vector>

namespace bxt::Presentation {

using CompareRequest = std::vector<SectionRequest>;

using LocationMap = std::unordered_map<std::string, std::string>;
using SectionMap = std::unordered_map<std::string, LocationMap>;

struct CompareResponse {
    std::vector<Core::Application::PackageSectionDTO> sections;
    std::unordered_map<std::string, SectionMap> compare_table;
};
} // namespace bxt::Presentation
