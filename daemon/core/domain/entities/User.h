/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/value_objects/Name.h"
#include "core/domain/value_objects/Permission.h"

#include <set>
#include <string>

namespace bxt::Core::Domain {

class User {
public:
    User() : m_name("Unnamed") {}

    User(const Name &name, const std::string &password)
        : m_name(name), m_password(password) {};

    const Name &id() const { return m_name; }
    const Name &name() const { return m_name; }

    const std::string &password() const { return m_password; }

    const std::set<Permission> &permissions() const { return m_permissions; }

    void set_name(const std::string &new_name) { m_name = new_name; }

    void set_password(const std::string &new_password) {
        m_password = new_password;
    }

    void set_permissions(const std::set<Permission> &new_permissions) {
        m_permissions = new_permissions;
    }

private:
    Name m_name;
    std::string m_password;
    std::set<Permission> m_permissions;
};

} // namespace bxt::Core::Domain
