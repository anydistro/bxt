/* === cpperr - Modern C++ Error Handling ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: MIT
 *
 */
#pragma once

#include "Factory.hpp"
#include <expected>
#include <utility>

namespace cpperr {

/**
 * Propagate errors with the same error type (Rust ? operator equivalent).
 * If the expression fails, returns the error immediately.
 * If the expression succeeds, extracts the value.
 * 
 * Usage:
 *   auto user = ERR_TRY(find_user(id));  // Returns error if find_user fails
 */
#define ERR_TRY(expr) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        return std::unexpected(__cpperr_result.error()); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

/**
 * Coroutine version of ERR_TRY.
 * Uses co_return instead of return.
 * 
 * Usage:
 *   auto user = ERR_CO_TRY(co_await async_find_user(id));
 */
#define ERR_CO_TRY(expr) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        co_return std::unexpected(__cpperr_result.error()); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

/**
 * Propagate errors with manual error type conversion.
 * If the expression fails, wraps the error with the specified target kind.
 * 
 * Usage:
 *   auto data = ERR_TRY_WRAP(db.query(id), HighLevelError::DatabaseError);
 */
#define ERR_TRY_WRAP(expr, target_kind) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        return ::cpperr::wrap_error(target_kind, __cpperr_result.error()); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

/**
 * Coroutine version of ERR_TRY_WRAP.
 * 
 * Usage:
 *   auto data = ERR_CO_TRY_WRAP(co_await db.async_query(id), HighLevelError::DatabaseError);
 */
#define ERR_CO_TRY_WRAP(expr, target_kind) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        co_return ::cpperr::wrap_error(target_kind, __cpperr_result.error()); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

/**
 * Propagate errors with manual wrapping and context.
 * 
 * Usage:
 *   auto data = ERR_TRY_WRAP_CTX(db.query(id), HighLevelError::DatabaseError, "fetching user");
 */
#define ERR_TRY_WRAP_CTX(expr, target_kind, context) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        return ::cpperr::wrap_error(target_kind, __cpperr_result.error(), context); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

/**
 * Coroutine version of ERR_TRY_WRAP_CTX.
 * 
 * Usage:
 *   auto data = ERR_CO_TRY_WRAP_CTX(co_await db.async_query(id), HighLevelError::DatabaseError, "fetching user");
 */
#define ERR_CO_TRY_WRAP_CTX(expr, target_kind, context) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        co_return ::cpperr::wrap_error(target_kind, __cpperr_result.error(), context); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

/**
 * Propagate errors with automatic error type conversion.
 * Uses convert_error function to convert source error kind to target kind.
 * Requires convert_error<TTarget>(TSource) to be defined.
 * 
 * Usage:
 *   auto data = ERR_TRY_AUTO(db.query(id), HighLevelError);
 */
#define ERR_TRY_AUTO(expr, TTarget) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        return ::cpperr::wrap_error<TTarget>(__cpperr_result.error()); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

/**
 * Coroutine version of ERR_TRY_AUTO.
 * 
 * Usage:
 *   auto data = ERR_CO_TRY_AUTO(co_await db.async_query(id), HighLevelError);
 */
#define ERR_CO_TRY_AUTO(expr, TTarget) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        co_return ::cpperr::wrap_error<TTarget>(__cpperr_result.error()); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

/**
 * Propagate errors with automatic wrapping and context.
 * 
 * Usage:
 *   auto data = ERR_TRY_AUTO_CTX(db.query(id), HighLevelError, "fetching user");
 */
#define ERR_TRY_AUTO_CTX(expr, TTarget, context) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        return ::cpperr::wrap_error<TTarget>(__cpperr_result.error(), context); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

/**
 * Coroutine version of ERR_TRY_AUTO_CTX.
 * 
 * Usage:
 *   auto data = ERR_CO_TRY_AUTO_CTX(co_await db.async_query(id), HighLevelError, "fetching user");
 */
#define ERR_CO_TRY_AUTO_CTX(expr, TTarget, context) ({ \
    auto&& __cpperr_result = (expr); \
    if (!__cpperr_result) { \
        co_return ::cpperr::wrap_error<TTarget>(__cpperr_result.error(), context); \
    } \
    std::forward<decltype(__cpperr_result)>(__cpperr_result).value(); \
})

} // namespace cpperr
