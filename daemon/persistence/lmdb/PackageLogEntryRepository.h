/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageLogEntryDTO.h"
#include "core/domain/entities/PackageLogEntry.h"
#include "persistence/lmdb/LmdbRepository.h"

namespace bxt::Persistence::LMDB {
using PackageLogEntryRepository = LmdbRepositoryBase<Core::Domain::PackageLogEntry, Core::Application::PackageLogEntryDTO>;
}
