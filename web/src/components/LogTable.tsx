/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { CommitRow } from "./logs/rows/CommitRow";
import { SyncRow } from "./logs/rows/SyncRow";
import { DeployRow } from "./logs/rows/DeployRow";
import {
    CommitLogEntry,
    DeployLogEntry,
    LogEntry,
    SyncLogEntry
} from "../definitions/log";
import { Card, Divider } from "react-daisyui";
import { useCallback } from "react";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import * as datefns from "date-fns";

type LogTableProps = {
    allEntries: LogEntry[];
    onRowClick: (entry: LogEntry) => void;
};

const LogTable = ({ allEntries, onRowClick }: LogTableProps) => {
    const renderContent = useCallback(
        (entry: (typeof allEntries)[0], index: number) => {
            switch (entry.type) {
                case "Commit":
                    return (
                        <CommitRow
                            key={index}
                            entry={entry as CommitLogEntry}
                        />
                    );
                case "Sync":
                    return (
                        <SyncRow key={index} entry={entry as SyncLogEntry} />
                    );
                case "Deploy":
                    return (
                        <DeployRow
                            key={index}
                            entry={entry as DeployLogEntry}
                        />
                    );
            }
        },
        []
    );

    return (
        <>
            {allEntries.map((entry, index) => (
                <Card
                    className={`text-sm transition bg-base-100 shadow-sm my-1 mx-10 cursor-pointer duration-150 hover:bg-base-300 hover:shadow-none ${entry.type === "Sync" ? "bg-secondary text-accent-content" : ""}`}
                    key={index}
                    onClick={() => {
                        onRowClick(entry);
                    }}
                >
                    <Card.Body className="p-4">
                        <div className="flex justify-between items-center gap-3">
                            <span
                                className={`flex text-sm w-44 ${entry.type === "Sync" ? "text-accent-content" : "text-gray-500"}`}
                            >
                                <time className="text-center">
                                    {datefns.format(
                                        entry.time,
                                        "yyyy-MM-dd HH:mm:ss"
                                    )}
                                </time>
                                <div className="grow" />
                            </span>
                            <Divider horizontal className="mx-1" />
                            <span className="flex items-center w-24">
                                <FontAwesomeIcon
                                    className="w-5"
                                    icon={entry.icon}
                                />
                                <h2 className="w-16 text-center align-middle">
                                    {entry.type}
                                </h2>
                            </span>
                            <Divider horizontal className="mx-1" />
                            {renderContent(entry, index)}
                        </div>
                    </Card.Body>
                </Card>
            ))}
        </>
    );
};

export default LogTable;
