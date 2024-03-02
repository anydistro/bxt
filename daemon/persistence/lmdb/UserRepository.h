/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/UserDTO.h"
#include "core/domain/entities/User.h"
#include "persistence/lmdb/LmdbRepository.h"
namespace bxt::Persistence::LMDB {

using UserRepository = LmdbRepositoryBase<User, UserDTO>;

} // namespace bxt::Persistence::LMDB
