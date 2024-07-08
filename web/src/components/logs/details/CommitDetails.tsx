/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Badge, Divider, Modal, ModalProps, Table } from "react-daisyui";
import {
    IconDefinition,
    faArrowsUpDownLeftRight,
    faCircle,
    faCirclePlus,
    faClock,
    faCodeCommit,
    faCopy,
    faCube,
    faTrash,
    faTrashCan,
    faUser
} from "@fortawesome/free-solid-svg-icons";
import { forwardRef, useEffect, useState } from "react";
import { createPortal } from "react-dom";
import { CommitLogEntry } from "../../../definitions/log";
import { formatDate } from "date-fns";
import SectionLabel from "../../SectionLabel";

export const CommitDetails = ({
    commitLogEntry
}: {
    commitLogEntry: CommitLogEntry;
}) => {
    const [commit, setCommit] = useState<CommitLogEntry>();

    useEffect(() => setCommit(commitLogEntry), [commitLogEntry]);

    return (
        <>
            <Modal.Header className="flex items-center gap-2">
                <FontAwesomeIcon icon={faCodeCommit} />
                <span>Commit</span>
                <Divider horizontal />

                {(
                    [
                        [commitLogEntry.added, faCirclePlus],
                        [commitLogEntry.deleted, faTrashCan],
                        [commitLogEntry.copied, faCopy],
                        [commitLogEntry.moved, faArrowsUpDownLeftRight]
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
                        {commitLogEntry.commiterUsername}
                    </span>
                    <Divider horizontal />

                    <time className="flex items-center gap-2">
                        <FontAwesomeIcon icon={faClock} />
                        {formatDate(commitLogEntry.time, "yyyy-MM-dd HH:mm:ss")}
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
                        {commit?.added.map((entry, index) => (
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
                        {commit?.deleted.map((entry, index) => (
                            <Table.Row key={`deleted-${index}`}>
                                <span>{entry.type}</span>
                                <span className="flex items-center">
                                    <FontAwesomeIcon
                                        icon={faTrashCan}
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
                        {commit?.moved.map(([from, to], index) => (
                            <Table.Row key={`moved-${index}`}>
                                <span>{from.type}</span>
                                <span className="flex items-center">
                                    <FontAwesomeIcon
                                        icon={faCube}
                                        className="px-2 max-h-6"
                                    />
                                    {from.name} {"->"} {to.name}
                                </span>
                                <span>
                                    <SectionLabel section={from.section} />{" "}
                                    {"->"} <SectionLabel section={to.section} />
                                </span>
                                <span>
                                    {from.version} {"->"} {to.version}
                                </span>
                            </Table.Row>
                        ))}
                        {commit?.copied.map(([from, to], index) => (
                            <Table.Row key={`copied-${index}`}>
                                <span>{from.type}</span>
                                <span className="flex items-center">
                                    <FontAwesomeIcon
                                        icon={faCube}
                                        className="px-2 max-h-6"
                                    />
                                    {from.name} {"->"} {to.name}
                                </span>
                                <span>
                                    <SectionLabel section={from.section} />{" "}
                                    {"->"} <SectionLabel section={to.section} />
                                </span>
                                <span>
                                    {from.version} {"->"} {to.version}
                                </span>
                            </Table.Row>
                        ))}
                    </Table.Body>
                </Table>
            </Modal.Body>
        </>
    );
};

export default CommitDetails;
