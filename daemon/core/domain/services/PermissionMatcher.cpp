/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PermissionMatcher.h"

#include <algorithm>

namespace bxt::Core::Domain::PermissionMatcher {

bool match(Permission const& lh, Permission const& rh) {
    auto const ltags = lh.tags();
    auto const rtags = rh.tags();

    auto const min = std::min(ltags.size(), rtags.size());

    for (std::size_t i = 0; i < min; i++) {
        if (ltags[i] == "*" || rtags[i] == "*") {
            continue;
        }
        if (ltags[i] != rtags[i]) {
            return false;
        }
    }

    if (ltags.size() != rtags.size() && !std::ranges::contains(ltags, "*")
        && !std::ranges::contains(rtags, "*")) {
        return false;
    }

    return true;
}

} // namespace bxt::Core::Domain::PermissionMatcher
