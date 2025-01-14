/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "MaintenanceService.h"

#include "core/application/dtos/PackageSectionDTO.h"

coro::task<void> bxt::Core::Application::MaintenanceService::export_database(
    std::set<PackageSectionDTO> const& sections) {
    auto available_sections = co_await m_section_service.get_sections();

    if (!available_sections.has_value()) {
        co_return;
    }

    std::set<PackageSectionDTO> sections_to_export;

    if (sections.empty()) {
        sections_to_export = *available_sections | std::ranges::to<std::set>();
    } else {
        for (auto const& section : sections) {
            if (std::find(available_sections->begin(), available_sections->end(), section)
                != available_sections->end()) {
                co_return;
            }
        }
    }

    m_exporter.add_dirty_sections(std::move(sections_to_export));

    co_await m_exporter.export_to_disk();

    co_return;
}
