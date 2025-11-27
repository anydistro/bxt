/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

// Core domain concepts
#include "core/AggregateRoot.hpp"
#include "core/AggregateReflector.hpp"
#include "core/DomainEvent.hpp"
#include "core/Repository.hpp"
#include "core/UnitOfWork.hpp"
#include "core/enum_string.hpp"

// Event bus - use eventpp library
// #include <eventpp/eventdispatcher.h>
// using EventBus = eventpp::EventDispatcher<std::type_index, void(DomainEvent const&)>;

// SQLite/sqlgen adapter (optional - only include if using)
// #include "adapters/sqlgen/SqlgenRepository.hpp"
// #include "adapters/sqlgen/SqlgenUnitOfWork.hpp"
// #include "adapters/sqlgen/SqlgenUnitOfWorkFactory.hpp"
