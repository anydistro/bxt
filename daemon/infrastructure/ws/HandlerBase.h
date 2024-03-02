/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include <coro/task.hpp>
#include <functional>
#include <json/value.h>

namespace bxt::Infrastructure {
struct HandlerBase {
    virtual ~HandlerBase() = default;
    virtual Json::Value handle_connection() {};
    virtual Json::Value handle_message(Json::Value message) {};
    virtual void set_callback(std::function<void(const Json::Value&)> cb) {
        m_callback = cb;
    };

    virtual std::string name() = 0;

protected:
    std::function<void(const Json::Value&)> m_callback;
};
} // namespace bxt::Infrastructure
