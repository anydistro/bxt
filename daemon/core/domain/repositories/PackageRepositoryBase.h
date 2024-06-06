/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/repositories/UnitOfWorkBase.h"
#include "coro/sync_wait.hpp"
#include "coro/task.hpp"

#include <core/domain/entities/Package.h>
#include <core/domain/repositories/RepositoryBase.h>
#include <functional>

namespace bxt::Core::Domain {
struct PackageRepositoryBase : public ReadWriteRepositoryBase<Package> {
    template<typename T>
    using ReadResult = ReadOnlyRepositoryBase<Package>::Result<T>;
    template<typename T>
    using WriteResult = ReadWriteRepositoryBase<Package>::Result<T>;

    virtual coro::task<TResults>
        find_by_section_async(const Section section,
                              std::shared_ptr<UnitOfWorkBase> uow) = 0;

    virtual coro::task<TResults> find_by_section_async(
        const Section section,
        const std::function<bool(const Package& pkg)> predicate,
        std::shared_ptr<UnitOfWorkBase> uow) = 0;

    virtual coro::task<TResult>
        find_by_section_async(const Section section,
                              const Name name,
                              std::shared_ptr<UnitOfWorkBase> uow) = 0;
};
} // namespace bxt::Core::Domain
