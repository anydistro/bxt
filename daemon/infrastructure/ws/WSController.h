/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "dexode/EventBus.hpp"
#include "drogon/WebSocketConnection.h"
#include "infrastructure/ws/HandlerBase.h"
#include "parallel_hashmap/phmap.h"

#include <drogon/WebSocketController.h>
#include <memory>
#include <vector>

namespace bxt::Infrastructure {
class WSController : public drogon::WebSocketController<WSController, false> {
public:
    WSController(std::shared_ptr<dexode::EventBus>);

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/api/ws");
    WS_PATH_LIST_END

    virtual void
        handleNewMessage(const drogon::WebSocketConnectionPtr &,
                         std::string &&,
                         const drogon::WebSocketMessageType &) override;
    virtual void
        handleNewConnection(const drogon::HttpRequestPtr &,
                            const drogon::WebSocketConnectionPtr &) override;
    virtual void
        handleConnectionClosed(const drogon::WebSocketConnectionPtr &) override;

private:
    phmap::parallel_node_hash_set<drogon::WebSocketConnectionPtr> m_connections;
    phmap::parallel_node_hash_map<std::string, std::shared_ptr<HandlerBase>>
        m_handlers;
};
} // namespace bxt::Infrastructure
