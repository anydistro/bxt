/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
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
