/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <expected>
#include <string_view>
#include <vector>

#include <coro/task.hpp>

#include "enum_string.hpp"

namespace percpptency {

// CRUD errors that repositories can return
enum class CrudError {
    NotFound,
    AlreadyExists,
    InvalidData,
    ConstraintViolation,
    ConcurrencyConflict,
    StorageError
};

// to_string provided by enum_string.hpp generic template

// Repository interface - adapters implement this
// TEntity is any struct reflectable by rfl::Reflector<TEntity>
template<typename TEntity, typename TId = std::string>
class Repository {
public:
    using ResultVoid = std::expected<void, CrudError>;
    using Result = std::expected<TEntity, CrudError>;
    using ResultVec = std::expected<std::vector<TEntity>, CrudError>;
    using ResultBool = std::expected<bool, CrudError>;
    using ResultCount = std::expected<std::size_t, CrudError>;

    virtual ~Repository() = default;

    virtual Result get_by_id(TId id) = 0;
    virtual ResultVec all() = 0;
    virtual ResultCount count() = 0;
    virtual ResultVoid create(TEntity& entity) = 0;
    virtual ResultVoid update(TEntity& entity) = 0;
    virtual ResultBool delete_by_id(TId id) = 0;
};

} // namespace percpptency
