/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"

#include <filesystem>

namespace bxt::Infrastructure {

class PackageFile {
public:
    PackageFile(const Core::Application::PackageSectionDTO& section,
                const std::filesystem::path& file_path)
        : m_section(section), m_file_path(file_path) {}

    PackageFile() = default;

    Core::Application::PackageSectionDTO section() const { return m_section; }

    std::filesystem::path file_path() const { return m_file_path; }

private:
    Core::Application::PackageSectionDTO m_section;
    std::filesystem::path m_file_path;
};

} // namespace bxt::Infrastructure
