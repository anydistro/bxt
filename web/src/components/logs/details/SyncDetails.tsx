/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Badge, Divider, Modal, Table } from "react-daisyui";
import {
    IconDefinition,
    faCirclePlus,
    faClock,
    faCube,
    faRotate,
    faTrashCan,
    faUser
} from "@fortawesome/free-solid-svg-icons";
import { useEffect, useState } from "react";
import { SyncLogEntry } from "../../../definitions/log";
import { formatDate } from "date-fns";
import SectionLabel from "../../SectionLabel";

export const SyncDetails = ({
    syncLogEntry
}: {
    syncLogEntry: SyncLogEntry;
}) => {
    const [sync, setSync] = useState<SyncLogEntry>();

    useEffect(() => setSync(syncLogEntry), [syncLogEntry]);
    return (
        <>
            <Modal.Header className="flex items-center gap-2">
                <span className="flex items-center gap-2">
                    <FontAwesomeIcon icon={faRotate} />
                    Sync
                </span>

                <Divider horizontal />

                {(
                    [
                        [syncLogEntry.added, faCirclePlus],
                        [syncLogEntry.deleted, faTrashCan]
                    ] as [Array<unknown>, IconDefinition][]
                ).map(
                    ([value, icon], index) =>
                        value.length > 0 && (
                            <span
                                key={index}
                                className="flex items-center gap-2 text-sm"
                            >
                                <FontAwesomeIcon icon={icon} />
                                {value.length}
                            </span>
                        )
                )}

                <div className="grow" />
                <Badge
                    className="flex items-center gap-2 text-sm"
                    color="secondary"
                    variant="outline"
                >
                    <span className="flex items-center gap-2">
                        <FontAwesomeIcon icon={faUser} />
                        {syncLogEntry.syncTriggerUsername}
                    </span>
                    <Divider horizontal />

                    <time className="flex items-center gap-2">
                        <FontAwesomeIcon icon={faClock} />
                        {formatDate(syncLogEntry.time, "yyyy-MM-dd HH:mm:ss")}
                    </time>
                </Badge>
            </Modal.Header>
            <Modal.Body>
                <Table zebra={true} size="xs" className="overflow-x-auto">
                    <Table.Head>
                        <span>Type</span>
                        <span>Name</span>
                        <span>Section</span>
                        <span>Version</span>
                    </Table.Head>
                    <Table.Body>
                        {sync?.added.map((entry, index) => (
                            <Table.Row key={`added-${index}`}>
                                <span>{entry.type}</span>
                                <span className="flex items-center">
                                    <FontAwesomeIcon
                                        icon={faCube}
                                        className="px-2 max-h-6"
                                    />
                                    {entry.name}
                                </span>
                                <span>
                                    <SectionLabel section={entry.section} />
                                </span>
                                <span>{entry.version}</span>
                            </Table.Row>
                        ))}
                        {sync?.deleted.map((entry, index) => (
                            <Table.Row key={`deleted-${index}`}>
                                <span>{entry.type}</span>
                                <span className="flex items-center">
                                    <FontAwesomeIcon
                                        icon={faTrashCan}
                                        className="px-2 max-h-6"
                                    />
                                    {entry.name}
                                </span>
                                <SectionLabel section={entry.section} />
                                <span>{entry.version}</span>
                            </Table.Row>
                        ))}
                    </Table.Body>
                </Table>
            </Modal.Body>
        </>
    );
};

export default SyncDetails;
