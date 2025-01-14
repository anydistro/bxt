/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/errors/CrudError.h"
#include "core/application/services/SectionService.h"
#include "infrastructure/PackageService.h"
#include "persistence/box/export/AlpmDBExporter.h"
#include "persistence/box/export/ExporterBase.h"
#include "utilities/errors/Macro.h"

#include <coro/task.hpp>

namespace bxt::Core::Application {
class MaintenanceService {
public:
    BXT_DECLARE_RESULT(CrudError)

    MaintenanceService(Persistence::Box::ExporterBase& exporter,
                       Application::SectionService& section_service)
        : m_exporter(exporter)
        , m_section_service(section_service) {
    }

    coro::task<void> export_database(std::set<PackageSectionDTO> const& sections = {});

private:
    Persistence::Box::ExporterBase& m_exporter;
    Application::SectionService& m_section_service;
};

} // namespace bxt::Core::Application
