/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/errors/CrudError.h"
#include "core/domain/entities/Package.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "frozen/unordered_map.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <coro/task.hpp>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace bxt::Core::Application {

class PackageService {
public:
    BXT_DECLARE_RESULT(CrudError);

    virtual ~PackageService() = default;

    struct Transaction {
        std::vector<PackageDTO> to_add;
        std::vector<std::pair<PackageSectionDTO, std::string>> to_remove;
    };

    virtual coro::task<Result<void>>
        commit_transaction(const Transaction transaction) = 0;

    virtual coro::task<Result<void>> add_package(const PackageDTO package) = 0;

    virtual coro::task<Result<std::vector<PackageDTO>>>
        get_packages(const PackageSectionDTO section_dto) const = 0;

    virtual coro::task<Result<void>>
        snap(const PackageSectionDTO from_section,
             const PackageSectionDTO to_section) = 0;
};

} // namespace bxt::Core::Application
