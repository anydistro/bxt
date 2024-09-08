/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/entities/User.h"
#include "core/domain/events/EventBase.h"
namespace bxt::Core::Domain::Events {

struct UserAdded : public EventBase {
    UserAdded(User const& user)
        : user(user) {
    }
    User user;

    virtual std::string message() const {
        return fmt::format("User \"{}\" added", user.name());
    }
};

struct UserRemoved : public EventBase {
    UserRemoved(Name const& id)
        : id(id) {
    }
    Name id;

    virtual std::string message() const {
        return fmt::format("User \"{}\" removed", std::string(id));
    }
};

struct UserUpdated : public EventBase {
    UserUpdated(User const& new_user, User const& old_user)
        : new_user(new_user)
        , old_user(old_user) {
    }

    User new_user;

    User old_user;

    virtual std::string message() const {
        return fmt::format("User \"{}\" updated", new_user.name());
    }
};

} // namespace bxt::Core::Domain::Events
