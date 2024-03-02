/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/configuration/Configuration.h"

#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <mailio/smtp.hpp>
#include <stdint.h>
#include <string>

namespace bxt::Infrastructure {
namespace {
    static constexpr frozen::
        unordered_map<frozen::string, mailio::smtps::auth_method_t, 3>
            AuthType = {
                {"none", mailio::smtps::auth_method_t::NONE},
                {"login", mailio::smtps::auth_method_t::LOGIN},
                {"starttls", mailio::smtps::auth_method_t::START_TLS},

    };
} // namespace

struct EmailOptions : public Utilities::Configurable {
    const std::string _section = "email";

    std::string server_hostname = "localhost";
    uint16_t server_port = 2222;

    mailio::smtps::auth_method_t connection_auth_type() {
        return AuthType.at(frozen::string {connection_auth_type_string.c_str(),
                                           connection_auth_type_string.size()});
    }

    std::string connection_auth_type_string = "none";
    std::string connection_user = "boxit";
    std::string connection_password = "1234";

    std::string sender_name = "bxt daemon";
    std::string sender_address = "daemon@bxt.local";

    // Configurable interface
public:
    virtual void serialize(Utilities::Configuration &config) override {
        config.set<std::string>("server-hostname", server_hostname);

        config.set<int64_t>("server-port", int64_t(server_port));

        config.set<std::string>("auth-type", connection_auth_type_string);

        config.set<std::string>("user", connection_user);
        config.set<std::string>("password", connection_password);

        config.set<std::string>("sender-name", sender_name);
        config.set<std::string>("sender-address", sender_address);
    }

    virtual void deserialize(const Utilities::Configuration &config) override {
        server_hostname = config.get<std::string>("server-hostname")
                              .value_or(server_hostname);

        server_port = config.get<int64_t>("server-port").value_or(server_port);

        connection_auth_type_string =
            config.get<std::string>("auth-type")
                .value_or(connection_auth_type_string);

        connection_user =
            config.get<std::string>("user").value_or(connection_user);
        connection_password =
            config.get<std::string>("password").value_or(connection_password);
        sender_name =
            config.get<std::string>("sender-name").value_or(sender_name);
        sender_address =
            config.get<std::string>("sender-address").value_or(sender_address);
    }
};

} // namespace bxt::Infrastructure
