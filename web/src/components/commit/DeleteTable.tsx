/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { faCube, faRotateLeft } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Button, Table } from "react-daisyui";
type DeleteTableProps = {
    action?: DeleteAction;
    restorePackage?: (name: string) => void;
};

export const DeleteTable = ({ action, restorePackage }: DeleteTableProps) => {
    return (
        <Table zebra={true} size="xs" className="overflow-x-auto">
            <Table.Head>
                <span>Name</span>
                <span />
                <span />
            </Table.Head>
            <Table.Body>
                {Array.from(action?.values() || []).map((name) => (
                    <Table.Row key={name}>
                        <span className="flex items-center">
                            <FontAwesomeIcon
                                icon={faCube}
                                className="px-2 max-h-6"
                            />
                            {name}
                        </span>
                        <span></span>
                        <Button
                            size="xs"
                            color="success"
                            onClick={(e) => {
                                e.preventDefault();
                                restorePackage?.(name);
                            }}
                        >
                            <FontAwesomeIcon
                                icon={faRotateLeft}
                                color="white"
                                className="max-h-6"
                            />
                        </Button>
                    </Table.Row>
                ))}
            </Table.Body>
        </Table>
    );
};

export default DeleteTable;
