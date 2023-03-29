/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: %YEAR% Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include <core/domain/value_objects/Permission.h>
#include <string>

namespace bxt::Core::Domain {

class PermissionMatcher {
public:
    bool match(const Permission &lh, const Permission &rh) const;
};

} // namespace bxt::Core::Domain