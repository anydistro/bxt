/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <optional>
#include <string>

#include <httplib.h>
namespace bxt::network {
struct NetworkError {
    httplib::Error error_code;
    std::optional<std::string> url;

    NetworkError(httplib::Error error, std::optional<std::string> url_path = std::nullopt)
        : error_code(error)
        , url(url_path) {
    }

    std::string what() const {
        std::string msg = "Network error: " + httplib::to_string(error_code);
        if (url) {
            msg += " (URL: " + *url + ")";
        }
        return msg;
    }
};
} // namespace bxt::network
