/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/UserDTO.h"
#include "core/application/errors/CrudError.h"
#include "core/domain/repositories/UserRepository.h"
#include "core/domain/services/PermissionMatcher.h"
#include "core/domain/value_objects/Permission.h"
#include "utilities/errors/Macro.h"

#include <string>
#include <string_view>
#include <vector>

namespace bxt::Core::Application {
class PermissionService {
public:
    BXT_DECLARE_RESULT(CrudError)
    PermissionService(Core::Domain::UserRepository& repository)
        : m_repository(repository) {}

    coro::task<bool> check(const std::string_view target_permission,
                           const std::string user_name);
    coro::task<bool>
        check(const std::vector<std::string_view> target_permissions,
              const std::string user_name);

private:
    Core::Domain::UserRepository& m_repository;
};

} // namespace bxt::Core::Application
