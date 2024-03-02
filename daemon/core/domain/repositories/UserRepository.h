/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/entities/User.h"
#include "core/domain/repositories/RepositoryBase.h"

namespace bxt::Core::Domain {
using UserRepository = ReadWriteRepositoryBase<User>;
} // namespace bxt::Core::Domain
