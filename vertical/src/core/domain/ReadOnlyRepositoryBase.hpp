/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <expected>

#include <coro/generator.hpp>

#include "core/result_types.hpp"
#include "core/error/kinds.hpp"

namespace bxt::domain::repo {

template<typename TEntity, typename TId> class ReadOnlyRepositoryBase {
    using CrudError = err::CrudError;

public:
    virtual ~ReadOnlyRepositoryBase() = default;

    virtual ResultTask<TEntity, CrudError> get_by_id_async(TId id) = 0;

    virtual ResultTask<Generator<TEntity>, CrudError>
        all_async(std::optional<Predicate<TEntity>> predicate = std::nullopt) = 0;

    virtual ResultTask<std::size_t, CrudError>
        count_async(std::optional<Predicate<TEntity>> predicate = std::nullopt) = 0;
};
} // namespace bxt::domain::repo
