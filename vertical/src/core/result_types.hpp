/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <coro/coro.hpp>
#include <coro/task.hpp>
#include <functional>

#include <cpperr/Error.hpp>
#include <cpperr/Factory.hpp>
#include <cpperr/Macros.hpp>

namespace bxt {

// Pure coroutine types (no framework dependencies)
template<typename T> using Task = coro::task<T>;
template<typename T> using Generator = coro::generator<T>;

// Re-export Result type from cpperr
template<typename T, typename E> 
using Result = cpperr::result<T, E>;

template<typename E> 
using VoidResult = cpperr::result<void, E>;

// Result types for coroutines
template<typename T, typename E> 
using ResultTask = coro::task<cpperr::result<T, E>>;

template<typename E> 
using VoidResultTask = coro::task<cpperr::result<void, E>>;

template<typename T, typename E> 
using GeneratorResult = cpperr::result<Generator<T>, E>;

// Predicates
template<typename T> 
using Predicate = std::function<bool(T const&)>;

} // namespace bxt
