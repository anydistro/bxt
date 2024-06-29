/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Button, Table } from "react-daisyui";
import {
    faCube,
    faSignature,
    faTrashCan
} from "@fortawesome/free-solid-svg-icons";

type CommitTableProps = {
    action?: AddAction;
    deletePackage?: (name: string) => void;
};

export const AddTable = ({ action, deletePackage }: CommitTableProps) => {
    return (
        <Table zebra={true} size="xs" className="overflow-x-auto">
            <Table.Head>
                <span>Name</span>
                <span />
                <span />
            </Table.Head>
            <Table.Body>
                {Array.from(action || []).map(([name, upload]) => (
                    <Table.Row key={name}>
                        <span className="flex items-center">
                            {upload.file !== undefined && (
                                <FontAwesomeIcon
                                    icon={faCube}
                                    className="px-2 max-h-6"
                                />
                            )}
                            {upload.signatureFile !== undefined && (
                                <FontAwesomeIcon
                                    icon={faSignature}
                                    color="green"
                                    className="px-2 max-h-6"
                                />
                            )}
                            {name}
                        </span>
                        <span></span>
                        <span>
                            <Button
                                size="xs"
                                color="error"
                                onClick={(e) => {
                                    e.preventDefault();
                                    deletePackage?.(name);
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
export default AddTable;
