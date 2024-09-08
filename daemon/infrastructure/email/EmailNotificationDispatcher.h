/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/notifications/NotificationDispatcherBase.h"
#include "infrastructure/email/EmailOptions.h"

#include <mailio/smtp.hpp>

namespace bxt::Infrastructure {

class EmailNotificationDispatcher : public Core::Application::NotificationDispatcherBase {
public:
    EmailNotificationDispatcher();
    virtual void dispatch(Core::Application::Notification const& notification) override;

private:
    EmailOptions options;
    mailio::smtps client;
};

} // namespace bxt::Infrastructure
