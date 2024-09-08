/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/services/PermissionMatcher.h"
#include "core/domain/value_objects/Name.h"
#include "core/domain/value_objects/Permission.h"

#include <set>
#include <string>

namespace bxt::Core::Domain {

class User {
public:
    User()
        : m_name("Unnamed") {
    }

    User(Name const& name, std::string const& password)
        : m_name(name)
        , m_password(password) {};

    Name const& id() const {
        return m_name;
    }
    Name const& name() const {
        return m_name;
    }

    std::string const& password() const {
        return m_password;
    }

    std::set<Permission> const& permissions() const {
        return m_permissions;
    }

    void set_name(std::string const& new_name) {
        m_name = new_name;
    }

    void set_password(std::string const& new_password) {
        m_password = new_password;
    }

    void set_permissions(std::set<Permission> const& new_permissions) {
        m_permissions = new_permissions;
    }

    bool has_permission(std::string_view target_permission) {
        for (auto const& permission : permissions()) {
            if (Core::Domain::PermissionMatcher::match(std::string(target_permission),
                                                       permission)) {
                return true;
            }
        }
        return false;
    }

private:
    Name m_name;
    std::string m_password;
    std::set<Permission> m_permissions;
};

} // namespace bxt::Core::Domain
