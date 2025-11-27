/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

// Re-export percpptency types for backward compatibility
#include "percpptency/core/DomainEvent.hpp"

namespace bxt::domain::events {

using percpptency::DomainEvent;

} // namespace bxt::domain::events
