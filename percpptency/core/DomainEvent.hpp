/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <chrono>
#include <string>

namespace percpptency {

// Plain struct - no virtual methods, just data
// Concrete events inherit from this to get timestamp and id
struct DomainEvent {
    std::chrono::system_clock::time_point occurred_at
        = std::chrono::system_clock::now();
    std::string event_id = generate_id();

private:
    static std::string generate_id();
};

} // namespace percpptency
