/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { forwardRef } from "react";
import { Badge, Modal, ModalProps, Table } from "react-daisyui";
import { createPortal } from "react-dom";
import SectionLabel from "../components/SectionLabel";

export type PackageModalProps = ModalProps & {
    package?: Package;
};

export const PackageModal = forwardRef<HTMLDialogElement, PackageModalProps>(
    (props, ref) => {
        return createPortal(
            <Modal ref={ref} {...props}>
                <Modal.Header>
                    <span className="font-bold">{props.package?.name}</span>
                    <Badge color="secondary" className="float-right">
                        <SectionLabel
                            className="space-x-4 text-sm"
                            section={props.package?.section}
                        />
                    </Badge>
                </Modal.Header>
                <Modal.Body>
                    <Table size="xs" zebra={true}>
                        <Table.Head>
                            <span>Candidate</span>
                            <span>Version</span>
                        </Table.Head>
                        {Object.entries(props.package?.poolEntries ?? []).map(
                            ([key, value]) => (
                                <Table.Row>
                                    <span>{key}</span>
                                    <span>{value.version}</span>
                                </Table.Row>
                            )
                        )}
                    </Table>
                </Modal.Body>
            </Modal>,
            document.body
        );
    }
);
export default PackageModal;
