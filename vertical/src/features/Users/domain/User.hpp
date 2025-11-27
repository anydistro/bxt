/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <set>
#include <string>
#include <utility>

#include <rfl/Result.hpp>

#include "Permission.hpp"
#include "core/result_types.hpp"
#include "core/types/Path.hpp"
#include "core/types/TimePoint.hpp"
#include "core/types/to_string.hpp"
#include "core/domain/value_objects/Name.hpp"
#include "core/domain/AggregateRoot.hpp"
#include "core/error/kinds.hpp"

namespace bxt::Users::domain {

using namespace ::bxt::domain;

enum class UserError {
    InvalidPassword,
    InvalidPermission,
    PermissionAlreadyExists,
    PermissionNotFound
};

inline std::string_view to_string(UserError e) {
    switch (e) {
        case UserError::InvalidPassword: return "InvalidPassword";
        case UserError::InvalidPermission: return "InvalidPermission";
        case UserError::PermissionAlreadyExists: return "PermissionAlreadyExists";
        case UserError::PermissionNotFound: return "PermissionNotFound";
    }
    return "Unknown";
}

// Domain events
struct UserCreated {
    value_objects::Name user_name;
    TimePoint created_at;
};

struct UserPasswordChanged {
    value_objects::Name user_name;
    TimePoint changed_at;
};

struct UserPermissionGranted {
    value_objects::Name user_name;
    Permission permission;
    TimePoint granted_at;
};

struct UserPermissionRevoked {
    value_objects::Name user_name;
    Permission permission;
    TimePoint revoked_at;
};

class User : public AggregateRoot<User> {
public:
    struct Data {
        std::string id;
        value_objects::Name name {*value_objects::Name::create("Unknown")};
        std::string password;
        std::set<Permission> permissions;
        TimePoint created_at;
        TimePoint last_modified;
    };

    friend struct rfl::Reflector<User>;

    static Result<User, UserError> create(value_objects::Name name, std::string const& password) {
        if (auto validation_result = validate_password(password); !validation_result) {
            return cpperr::make_error(UserError::InvalidPassword);
        }

        auto now = std::chrono::system_clock::now();
        return User {std::move(name), password, now};
    }

    static User create_from_data(Data const& data) {
        User user;
        user.m_data = data;
        return user;
    }

    ~User() = default;

    value_objects::Name const& id() const {
        return m_data.name;
    }

    value_objects::Name const& name() const {
        return m_data.name;
    }

    std::string const& password() const {
        return m_data.password;
    }

    std::set<Permission> const& permissions() const {
        return m_data.permissions;
    }

    Result<void, UserError> change_password(std::string const& new_password) {
        if (auto validation_result = validate_password(new_password); !validation_result) {
            return cpperr::make_error(UserError::InvalidPassword);
        }

        m_data.password = new_password;
        m_data.last_modified = std::chrono::system_clock::now();

        return {};
    }

    Result<void, UserError> grant_permission(Permission const& permission) {
        if (m_data.permissions.contains(permission)) {
            return cpperr::make_error(UserError::PermissionAlreadyExists);
        }

        m_data.permissions.insert(permission);
        m_data.last_modified = std::chrono::system_clock::now();

        return {};
    }

    Result<void, UserError> revoke_permission(Permission const& permission) {
        if (!m_data.permissions.contains(permission)) {
            return cpperr::make_error(UserError::PermissionNotFound);
        }

        m_data.permissions.erase(permission);
        m_data.last_modified = std::chrono::system_clock::now();

        return {};
    }

    TimePoint created_at() const {
        return m_data.created_at;
    }

    TimePoint last_modified() const {
        return m_data.last_modified;
    }

    bool has_permission(Permission const& permission) const {
        return m_data.permissions.contains(permission);
    }

private:
    User() = default;

    User(value_objects::Name name, std::string password, TimePoint created_at) {
        m_data.id = name;
        m_data.name = std::move(name);
        m_data.password = std::move(password);
        m_data.created_at = created_at;
        m_data.last_modified = created_at;
    }

    static bool validate_password(std::string const& password) {
        return !password.empty();
    }

    Data m_data;
};

} // namespace bxt::Users::domain
