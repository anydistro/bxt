/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "coro/io_scheduler.hpp"

#include <coro/mutex.hpp>
#include <coro/shared_mutex.hpp>
namespace bxt::Utilities {
template<typename T> struct locked {
    coro::shared_scoped_lock<coro::io_scheduler> lock;

    T value;
};
} // namespace bxt::Utilities
