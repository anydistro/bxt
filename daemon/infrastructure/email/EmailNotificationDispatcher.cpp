/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "EmailNotificationDispatcher.h"

#include <mailio/message.hpp>

namespace bxt::Infrastructure {

EmailNotificationDispatcher::EmailNotificationDispatcher()
    : client(options.server_hostname, options.server_port) {
    client.authenticate(options.connection_user, options.connection_password,
                        options.connection_auth_type());
}

void EmailNotificationDispatcher::dispatch(
    const Core::Application::Notification &notification) {
    if (!notification.valid()) { return; }

    mailio::message msg;

    msg.subject(notification.title);
    msg.content(notification.content);

    msg.sender({options.sender_name, options.sender_address});

    client.submit(msg);
}

} // namespace bxt::Infrastructure
