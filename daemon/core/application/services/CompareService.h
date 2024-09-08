/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "core/application/dtos/PackageDTO.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "core/application/errors/CompareError.h"
#include "core/application/errors/CrudError.h"
#include "core/application/services/PackageService.h"
#include "core/domain/entities/Package.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "frozen/unordered_map.h"
#include "parallel_hashmap/phmap.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <coro/task.hpp>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace bxt::Core::Application {

class CompareService {
public:
    BXT_DECLARE_RESULT(CompareError);

    CompareService(PackageService& package_service)
        : m_package_service(package_service) {
    }

    virtual ~CompareService() = default;

    struct CompareResult {
        std::vector<PackageSectionDTO> sections;
        phmap::flat_hash_map<std::tuple<std::string, PackageSectionDTO, Domain::PoolLocation>,
                             std::string>
            compare_table;
    };

    virtual coro::task<Result<CompareResult>>
        compare(std::vector<PackageSectionDTO> const sections);

private:
    PackageService& m_package_service;
};

} // namespace bxt::Core::Application
