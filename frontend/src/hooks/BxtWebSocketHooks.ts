/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { useState, useEffect } from "react";
import useWebSocket from "react-use-websocket";

export const useSyncMessage = () => {
    const [messages, setMessage] = useState<SyncMessage>();
    const socketUrl = `${
        window.location.protocol === "https:" ? "wss" : "ws"
    }://${window.location.host.toString()}/api/ws`;

    const { lastMessage, readyState } = useWebSocket(socketUrl);

    useEffect(() => {
        if (lastMessage !== null && readyState === WebSocket.OPEN) {
            try {
                const message: SyncMessage = JSON.parse(lastMessage.data);
                setMessage(message);
            } catch (error) {
                console.error("Failed to parse WebSocket message:", error);
            }
        }
    }, [lastMessage, readyState]);

    return messages;
};
