/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Button, Table } from "react-daisyui";
import { faCube, faTrashCan } from "@fortawesome/free-solid-svg-icons";
import SectionLabel from "../SectionLabel";

type TransferTableProps = {
    action?: TransferAction;
    removeTransfer?: (name: string) => void;
};

export const TransferTable = ({
    action,
    removeTransfer
}: TransferTableProps) => {
    return (
        <Table zebra={true} size="xs" className="overflow-x-auto">
            <Table.Head>
                <span>Name</span>
                <span>Target</span>
                <span />
            </Table.Head>
            <Table.Body>
                {Array.from(action || []).map(([name, section]) => (
                    <Table.Row key={name}>
                        <span className="flex items-center">
                            <FontAwesomeIcon
                                icon={faCube}
                                className="px-2 max-h-6"
                            />
                            {name}
                        </span>
                        <span>
                            <SectionLabel section={section} />
                        </span>
                        <span>
                            <Button
                                size="xs"
                                color="error"
                                onClick={(e) => {
                                    e.preventDefault();
                                    removeTransfer?.(name);
                                }}
                            >
                                <FontAwesomeIcon
                                    icon={faTrashCan}
                                    color="white"
                                    className="max-h-6"
                                />
                            </Button>
                        </span>
                    </Table.Row>
                ))}
            </Table.Body>
        </Table>
    );
};

export default TransferTable;
