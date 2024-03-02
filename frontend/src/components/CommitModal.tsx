/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Button, Form, Modal, ModalProps, Table } from "react-daisyui";
import {
    faCube,
    faSignature,
    faTrashCan
} from "@fortawesome/free-solid-svg-icons";
import { toast } from "react-toastify";
import "react-toastify/dist/ReactToastify.css";
import * as uuid from "uuid";
import { forwardRef, useCallback, useEffect, useState } from "react";
import { createPortal } from "react-dom";
import SectionSelect from "./SectionSelect";

export type CommitModalProps = ModalProps & {
    isNew?: boolean;
    commit?: ICommit;
    sections?: ISection[];
    onCommitSubmit?: (commit: ICommit) => void;
    onCommitDelete?: (id: string) => void;
};

export default forwardRef<HTMLDialogElement, CommitModalProps>(
    (props: CommitModalProps, ref) => {
        const [commit, setCommit] = useState<ICommit>(
            props.commit || { id: uuid.v4(), section: {}, packages: [] }
        );

        useEffect(
            () =>
                setCommit(
                    props.commit || { id: uuid.v4(), section: {}, packages: [] }
                ),
            [props.commit]
        );

        const { id, section, packages } = commit;

        const { branch, repository, architecture } = section;

        const commitCallback = useCallback(() => {
            toast.success("Commit created!");

            if (props.onCommitSubmit) props.onCommitSubmit(commit);
        }, [commit, props.onSubmit]);

        return createPortal(
            <Modal ref={ref} className="w-11/12 max-w-5xl" {...props}>
                <Modal.Header className="text-3xl font-bold">
                    Commit
                </Modal.Header>
                <Modal.Body>
                    <Form>
                        <Form.Label title="Section" />
                        <SectionSelect
                            sections={props.sections || []}
                            selectedSection={section}
                            className="w-full"
                        />
                        <Form.Label title="Packages" />

                        <Table
                            zebra={true}
                            size="xs"
                            className="overflow-x-auto"
                        >
                            <Table.Head>
                                <span>Name</span>
                                <span />
                            </Table.Head>
                            <Table.Body>
                                {props.commit?.packages.map((value) => {
                                    return (
                                        <Table.Row>
                                            <span className="flex items-center">
                                                <FontAwesomeIcon
                                                    icon={faCube}
                                                    className="px-2 max-h-6"
                                                />
                                                {value.signatureFile !==
                                                    undefined && (
                                                    <FontAwesomeIcon
                                                        icon={faSignature}
                                                        color="green"
                                                        className="px-2 max-h-6"
                                                    />
                                                )}

                                                {value.name}
                                            </span>
                                            <span></span>
                                        </Table.Row>
                                    );
                                })}
                            </Table.Body>
                        </Table>
                    </Form>
                </Modal.Body>

                <Form className="mt-6">
                    <span className="flex space-x-4 justify-end">
                        {props.isNew && (
                            <Button
                                startIcon={
                                    <FontAwesomeIcon
                                        className="px-1"
                                        icon={faTrashCan}
                                    />
                                }
                                onClick={(e) => {
                                    if (props.onCommitDelete)
                                        props.onCommitDelete(commit.id);
                                }}
                                type="button"
                                color="error"
                            >
                                Delete
                            </Button>
                        )}
                        <Button
                            onClick={commitCallback}
                            type="button"
                            color="primary"
                        >
                            Commit
                        </Button>
                    </span>
                </Form>
            </Modal>,
            document.body
        );
    }
);
