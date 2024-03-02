/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "WSController.h"

#include "drogon/HttpTypes.h"
#include "drogon/WebSocketConnection.h"
#include "infrastructure/ws/SyncProtocolHandler.h"

#include "json/value.h"
#include "json/writer.h"

namespace bxt::Infrastructure {
using namespace drogon;

WSController::WSController(std::shared_ptr<dexode::EventBus> evbus) {
    auto handler = std::make_shared<SyncHandler>(evbus);
    handler->set_callback([this](const auto &message) {
        Json::StreamWriterBuilder builder;

        Json::Value msg = message;
        msg["type"] = "sync";
        for (auto &connection : m_connections) {
            connection->send(Json::writeString(builder, msg));
        }
    });

    m_handlers.emplace("sync", handler);
}

void WSController::handleNewMessage(const WebSocketConnectionPtr &connection,
                                    std::string &&message,
                                    const WebSocketMessageType &type) {
    if (type == WebSocketMessageType::Ping) {
        connection->send("", WebSocketMessageType::Pong);
        return;
    }
    if (type == WebSocketMessageType::Pong) { return; }

    if (type != WebSocketMessageType::Text) {
        connection->send("This connection uses JSON messages");
        return;
    }

    Json::Reader reader;
    Json::Value message_json;

    if (!reader.parse(message, message_json)) {
        connection->send("This connection uses JSON messages");
        return;
    }

    const auto handler_type = message_json["type"].asString();
    Json::StreamWriterBuilder builder;

    const auto handler = m_handlers.find(handler_type);
    if (handler == m_handlers.end()) {
        Json::Value response;
        response["status"] = "error";
        response["message"] = "Invalid handler type";

        connection->send(Json::writeString(builder, response));
        return;
    }

    connection->send(Json::writeString(
        builder, handler->second->handle_message(message_json)));
}

void WSController::handleNewConnection(
    const HttpRequestPtr &req, const WebSocketConnectionPtr &connection) {
    Json::StreamWriterBuilder builder;

    for (const auto &handler : m_handlers) {
        connection->send(
            Json::writeString(builder, handler.second->handle_connection()));
    }
    m_connections.emplace(connection);
}

void WSController::handleConnectionClosed(
    const WebSocketConnectionPtr &connection) {
    m_connections.erase(connection);
}

} // namespace bxt::Infrastructure
