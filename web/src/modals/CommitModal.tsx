/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    Accordion,
    Badge,
    Button,
    Card,
    Form,
    Modal,
    ModalProps
} from "react-daisyui";
import {
    faArrowsAlt,
    faCirclePlus,
    faCopy,
    faTrashAlt,
    faTrashCan,
    faUpload
} from "@fortawesome/free-solid-svg-icons";
import "react-toastify/dist/ReactToastify.css";
import { forwardRef, useCallback, useEffect, useState } from "react";
import { createPortal } from "react-dom";
import SectionSelect from "../components/SectionSelect";
import { useDropzone } from "react-dropzone-esm";
import { usePackageDropHandler } from "../hooks/DragNDropHooks";
import AddTable from "../components/commit/AddTable";
import DeleteTable from "../components/commit/DeleteTable";
import TransferTable from "../components/commit/TransferTable";
import _ from "lodash";
import { createCommit, mergeCommits } from "../utils/CommitUtils";
import SectionLabel from "../components/SectionLabel";
import { useFilePicker } from "use-file-picker";

export type CommitModalProps = ModalProps & {
    isNew?: boolean;
    commit?: Commit;
    section: Section;
    sections?: Section[];
    onCommitSubmit?: (section: Section, commit: Commit) => void;
    onCommitDelete?: (section: Section | undefined) => void;
    onPackageDrop?: (files: File[]) => void;
    defaultCheckedAction?: ActionType;
};

export const CommitModal = forwardRef<HTMLDialogElement, CommitModalProps>(
    (props: CommitModalProps, ref) => {
        const [commit, setCommit] = useState<Commit>();
        const [section, setSection] = useState<Section>();

        useEffect(() => setCommit(props.commit), [props.commit, setCommit]);

        useEffect(() => setSection(props.section), [props.section, setSection]);

        const { toAdd, toDelete, toMove, toCopy } = commit || {};

        const commitCallback = useCallback(() => {
            if (props.onCommitSubmit && commit && section)
                props.onCommitSubmit(section, commit);
        }, [commit, props, section]);

        const packageDropHandler = usePackageDropHandler(section, (_, toAdd) =>
            setCommit((prevCommit) => {
                const commit = createCommit({ toAdd });

                const newCommit = prevCommit
                    ? mergeCommits(prevCommit, commit)
                    : commit;

                return newCommit;
            })
        );

        const { openFilePicker } = useFilePicker({
            multiple: true,
            onFilesSelected: ({ plainFiles }) => {
                packageDropHandler(plainFiles);
            }
        });

        const { getRootProps, getInputProps } = useDropzone({
            onDrop: packageDropHandler,
            noClick: true
        });

        return createPortal(
            <Modal ref={ref} className="w-11/12 max-w-5xl" {...props}>
                <Modal.Header>
                    <span>Commit</span>

                    <div className="float-right">
                        {props.isNew ? (
                            <SectionSelect
                                sections={props.sections || []}
                                selectedSection={section}
                                onSelected={(section) => {
                                    setSection(section);
                                }}
                            />
                        ) : (
                            <Badge color="secondary" className="text-white">
                                <SectionLabel section={section} />
                            </Badge>
                        )}
                    </div>
                </Modal.Header>
                <Modal.Body {...getRootProps()}>
                    <input {...getInputProps()} />
                    <Form>
                        {[
                            {
                                action: "add",
                                title: `To Add (${toAdd?.size || 0})`,
                                visible: !_.isEmpty(commit?.toAdd),
                                icon: faCirclePlus,
                                Component: (
                                    <AddTable
                                        action={commit?.toAdd}
                                        deletePackage={(name) => {
                                            setCommit((prevCommit) => {
                                                prevCommit?.toAdd?.delete(name);
                                                return createCommit(prevCommit);
                                            });
                                        }}
                                    />
                                )
                            },
                            {
                                action: "delete",
                                title: `To Delete (${toDelete?.size || 0})`,
                                visible: !_.isEmpty(commit?.toDelete),
                                icon: faTrashAlt,
                                Component: (
                                    <DeleteTable
                                        action={commit?.toDelete}
                                        restorePackage={(name) => {
                                            setCommit((prevCommit) => {
                                                if (
                                                    !prevCommit ||
                                                    !prevCommit.toDelete
                                                )
                                                    return prevCommit;

                                                prevCommit.toDelete.delete(
                                                    name
                                                );

                                                return createCommit(prevCommit);
                                            });
                                        }}
                                    />
                                )
                            },
                            {
                                action: "move",
                                title: `To Move (${toMove?.size || 0})`,
                                visible: !_.isEmpty(commit?.toMove),
                                icon: faArrowsAlt,
                                Component: (
                                    <TransferTable
                                        action={commit?.toMove}
                                        removeTransfer={(name) => {
                                            setCommit((prevCommit) => {
                                                if (
                                                    !prevCommit ||
                                                    !prevCommit.toMove
                                                )
                                                    return prevCommit;
                                                prevCommit.toMove.delete(name);
                                                return createCommit(prevCommit);
                                            });
                                        }}
                                    />
                                )
                            },
                            {
                                action: "copy",
                                title: `To Copy (${toCopy?.size || 0})`,
                                visible: !_.isEmpty(commit?.toCopy),
                                icon: faCopy,
                                Component: (
                                    <TransferTable
                                        action={commit?.toCopy}
                                        removeTransfer={(name) => {
                                            setCommit((prevCommit) => {
                                                if (
                                                    !prevCommit ||
                                                    !prevCommit.toCopy
                                                )
                                                    return prevCommit;
                                                prevCommit.toCopy.delete(name);
                                                return createCommit(prevCommit);
                                            });
                                        }}
                                    />
                                )
                            }
                        ].map(
                            (
                                { action, title, visible, icon, Component },
                                index
                            ) =>
                                visible && (
                                    <Card
                                        className="my-2 shadow-md"
                                        key={index}
                                    >
                                        <Accordion
                                            icon="arrow"
                                            defaultChecked={
                                                action ===
                                                props.defaultCheckedAction
                                            }
                                        >
                                            <Accordion.Title className="text-lg font-semibold">
                                                <FontAwesomeIcon
                                                    className="mr-2"
                                                    icon={icon}
                                                />
                                                {title}
                                            </Accordion.Title>
                                            <Accordion.Content>
                                                {Component}
                                            </Accordion.Content>
                                        </Accordion>
                                    </Card>
                                )
                        )}
                    </Form>
                </Modal.Body>
                <Form className="mt-6">
                    <span className="flex space-x-4 justify-end">
                        <Button
                            color="accent"
                            onClick={(e) => {
                                e.preventDefault();
                                openFilePicker();
                            }}
                        >
                            <FontAwesomeIcon icon={faUpload} />
                            Upload
                        </Button>
                        <div className="grow" />
                        {!props.isNew && (
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
