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
import { forwardRef, useCallback, useEffect, useState } from "react";
import { createPortal } from "react-dom";
import SectionSelect from "../components/SectionSelect";
import Dropzone from "react-dropzone-esm";
import { usePackageDropHandler } from "../hooks/DragNDropHooks";

export type CommitModalProps = ModalProps & {
    isNew?: boolean;
    commit?: Commit;
    section: Section;
    sections?: Section[];
    onCommitSubmit?: (section: Section, commit: Commit) => void;
    onCommitDelete?: (section: Section | undefined) => void;
    onPackageDrop?: (files: File[]) => void;
};

export const CommitModal = forwardRef<HTMLDialogElement, CommitModalProps>(
    (props: CommitModalProps, ref) => {
        const [commit, setCommit] = useState<Commit>();
        const [section, setSection] = useState<Section>();

        useEffect(() => setCommit(props.commit), [props.commit]);

        useEffect(() => setSection(props.section), [props.section]);

        useEffect(() => {
            if (commit?.size == 0) {
                props.onCommitDelete?.(section);
            }
        }, [commit, props.onCommitDelete, section]);

        const commitCallback = useCallback(() => {
            toast.success("Commit created!");

            if (props.onCommitSubmit && commit && section)
                props.onCommitSubmit(section, commit);
        }, [commit, props.onSubmit]);

        return createPortal(
            <Modal ref={ref} className="w-11/12 max-w-5xl" {...props}>
                <Modal.Header>
                    <span>Commit</span>
                    <div className="float-right">
                        <SectionSelect
                            sections={props.sections || []}
                            selectedSection={section}
                            onSelected={(section) => {
                                setSection(section);
                            }}
                        />
                    </div>
                </Modal.Header>
                <Dropzone
                    noClick={true}
                    onDrop={usePackageDropHandler(section, (_, commit) =>
                        setCommit((prevCommit) => {
                            const newCommit = new Map(prevCommit);

                            commit.forEach((value, key) => {
                                newCommit.set(key, {
                                    ...value,
                                    ...commit.get(key)
                                });
                            });

                            return newCommit;
                        })
                    )}
                >
                    {({ getRootProps, getInputProps }) => (
                        <Modal.Body {...getRootProps()}>
                            <input {...getInputProps()} />
                            <Form>
                                <Form.Label title="Packages" />

                                <Table
                                    zebra={true}
                                    size="xs"
                                    className="overflow-x-auto"
                                >
                                    <Table.Head>
                                        <span>Name</span>
                                        <span />
                                        <span />
                                    </Table.Head>
                                    <Table.Body>
                                        {Array.from(commit || []).map(
                                            ([name, upload]) => {
                                                return (
                                                    <Table.Row>
                                                        <span className="flex items-center">
                                                            {upload.file !==
                                                                undefined && (
                                                                <FontAwesomeIcon
                                                                    icon={
                                                                        faCube
                                                                    }
                                                                    className="px-2 max-h-6"
                                                                />
                                                            )}
                                                            {upload.signatureFile !==
                                                                undefined && (
                                                                <FontAwesomeIcon
                                                                    icon={
                                                                        faSignature
                                                                    }
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
                                                                onClick={() => {
                                                                    setCommit(
                                                                        (
                                                                            prevCommit
                                                                        ) => {
                                                                            const newCommit =
                                                                                new Map(
                                                                                    prevCommit
                                                                                );
                                                                            newCommit.delete(
                                                                                name
                                                                            );
                                                                            return newCommit;
                                                                        }
                                                                    );
                                                                }}
                                                            >
                                                                <FontAwesomeIcon
                                                                    icon={
                                                                        faTrashCan
                                                                    }
                                                                    color="white"
                                                                    className="max-h-6"
                                                                />
                                                            </Button>
                                                        </span>
                                                    </Table.Row>
                                                );
                                            }
                                        )}
                                    </Table.Body>
                                </Table>
                            </Form>
                        </Modal.Body>
                    )}
                </Dropzone>
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
                                        props.onCommitDelete(props.section);
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
export default CommitModal;
