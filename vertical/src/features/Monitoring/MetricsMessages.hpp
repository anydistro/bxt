/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <cstdint>

#include "infra/reflect/TimePointParser.hpp"
#include "core/result_types.hpp"
#include "core/types/Path.hpp"
#include "core/types/TimePoint.hpp"
#include "core/types/to_string.hpp"

namespace bxt::Monitoring {

struct MetricsResponse {
    TimePoint timestamp;
    // Application lifecycle
    uint64_t uptime_seconds = 0;
    TimePoint started_at;

    // Users & Authentication
    uint32_t total_users = 0;

    uint32_t active_sessions = 0;
    uint64_t auth_attempts_total = 0;
    uint64_t auth_failures_total = 0;

    // Package Repository Data
    uint64_t total_packages = 0;

    uint32_t total_sections = 0;
    uint32_t packages_with_signatures = 0;
    uint64_t total_package_size_bytes = 0;

    // Synchronization (from /api/packages/sync)
    uint64_t sync_operations_total = 0;
    uint64_t sync_failures_total = 0;
    TimePoint last_successful_sync;

    // Database metrics
    uint64_t database_size_bytes = 0;
    uint64_t database_entries = 0;

    // API Usage
    uint64_t api_requests_total = 0;
    uint64_t api_errors_total = 0;
};

} // namespace bxt::Monitoring
