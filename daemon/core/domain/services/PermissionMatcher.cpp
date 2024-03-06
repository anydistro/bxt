/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PermissionMatcher.h"

namespace bxt::Core::Domain::PermissionMatcher {

bool match(const Permission &lh, const Permission &rh) {
    const auto ltags = lh.tags();
    const auto rtags = rh.tags();

    const auto min = std::min(ltags.size(), rtags.size());

    for (std::size_t i = 0; i < min; i++) {
        if (ltags[i] == "*" || rtags[i] == "*") { continue; }
        if (ltags[i] != rtags[i]) { return false; }
    }

    return true;
}

} // namespace bxt::Core::Domain::PermissionMatcher
