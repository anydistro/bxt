/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <expected>

#include <coro/generator.hpp>

#include "ReadOnlyRepositoryBase.hpp"
#include "core/result_types.hpp"
#include "core/error/kinds.hpp"

namespace bxt::domain::repo {

template<typename TEntity, typename TId = std::string>
class RepositoryBase : public ReadOnlyRepositoryBase<TEntity, TId> {
    using CrudError = err::CrudError;

public:
    virtual ~RepositoryBase() = default;

    virtual ResultTask<TEntity, CrudError> create_async(TEntity const& entity) = 0;
    virtual ResultTask<TEntity, CrudError> update_async(TEntity const& entity) = 0;
    virtual ResultTask<bool, CrudError> delete_async(TId id) = 0;
};

} // namespace bxt::domain::repo
