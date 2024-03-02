/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/notifications/Notification.h"

namespace bxt::Core::Application {

class NotificationDispatcherBase {
public:
    virtual ~NotificationDispatcherBase() = default;

    virtual void dispatch(const Notification& notification) = 0;
};

} // namespace bxt::Core::Application
