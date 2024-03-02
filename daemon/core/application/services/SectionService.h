/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/errors/CrudError.h"
#include "core/domain/entities/Section.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "coro/task.hpp"
#include "utilities/errors/Macro.h"

#include <vector>

namespace bxt::Core::Application {

class SectionService {
public:
    BXT_DECLARE_RESULT(CrudError)
    SectionService(Domain::ReadOnlyRepositoryBase<Domain::Section>& repository)
        : m_repository(repository) {}

    coro::task<Result<std::vector<PackageSectionDTO>>> get_sections() const;

private:
    Domain::ReadOnlyRepositoryBase<Domain::Section>& m_repository;
};

} // namespace bxt::Core::Application
