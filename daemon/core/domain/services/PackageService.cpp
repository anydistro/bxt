/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "PackageService.h"

namespace bxt::Core::Domain {

coro::task<void> PackageService::add_package(Package pkg) {
}

coro::task<void> PackageService::remove_package(Package pkg) {
}

coro::task<void> PackageService::update_package(Package pkg, bool enable_downgrade) {
}

} // namespace bxt::Core::Domain
