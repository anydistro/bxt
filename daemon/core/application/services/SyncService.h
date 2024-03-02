/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"

#include <coro/task.hpp>

namespace bxt::Core::Application {

class SyncService {
public:
    virtual ~SyncService() = default;

    virtual coro::task<void> sync(const PackageSectionDTO section) = 0;
    virtual coro::task<void> sync_all() = 0;
};

} // namespace bxt::Core::Application
