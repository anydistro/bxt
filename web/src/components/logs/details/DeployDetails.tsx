/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Badge, Divider, Modal, Table } from "react-daisyui";
import {
    faCirclePlus,
    faClock,
    faCube,
    faRobot,
    faRocket
} from "@fortawesome/free-solid-svg-icons";
import { useEffect, useState } from "react";
import { DeployLogEntry } from "../../../definitions/log";
import { formatDate } from "date-fns";
import SectionLabel from "../../SectionLabel";

export const DeployDetails = ({
    deployLogEntry
}: {
    deployLogEntry: DeployLogEntry;
}) => {
    const [deploy, setDeploy] = useState<DeployLogEntry>();

    useEffect(() => setDeploy(deployLogEntry), [deployLogEntry]);
    return (
        <>
            <Modal.Header className="flex items-center gap-2">
                <FontAwesomeIcon icon={faRocket} />
                <span>Deployment</span>
                <Divider horizontal />
                {deployLogEntry.added.length > 0 && (
                    <span className="flex items-center gap-2 text-sm">
                        <FontAwesomeIcon icon={faCirclePlus} />
                        {deployLogEntry.added.length}
                    </span>
                )}

                <div className="grow" />

                <Badge
                    className="flex items-center gap-2 text-sm"
                    color="secondary"
                    variant="outline"
                >
                    <span className="flex items-center gap-2">
                        <FontAwesomeIcon icon={faRobot} />
                        {deployLogEntry.runnerUrl}
                    </span>
                    <Divider horizontal />

                    <time className="flex items-center gap-2">
                        <FontAwesomeIcon icon={faClock} />
                        {formatDate(deployLogEntry.time, "yyyy-MM-dd HH:mm:ss")}
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
                        {deploy?.added.map((entry, index) => (
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
                    </Table.Body>
                </Table>
            </Modal.Body>
        </>
    );
};

export default DeployDetails;
