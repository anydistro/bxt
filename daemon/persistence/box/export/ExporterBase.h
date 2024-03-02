/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"

#include <coro/task.hpp>
#include <set>
#include <string>

namespace bxt::Persistence::Box {
struct ExporterBase {
    virtual ~ExporterBase() = default;

    virtual coro::task<void> export_to_disk() = 0;
    virtual void add_dirty_sections(
        std::set<Core::Application::PackageSectionDTO>&&) = 0;
};
} // namespace bxt::Persistence::Box
