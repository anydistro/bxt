/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useCallback, useEffect, useMemo, useState } from "react";
import { faCodeCommit } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    FileBrowserHandle,
    FileBrowserProps,
    FullFileBrowser as F,
    setChonkyDefaults
} from "chonky";
import { Button, Loading } from "react-daisyui";
import Dropzone from "react-dropzone-esm";
import { usePushCommitsHandler, useSections } from "../hooks/BxtHooks";
import { usePackageDropHandler } from "../hooks/DragNDropHooks";
import {
    useFileActionHandler,
    useFolderChainForPath
} from "../hooks/FileManagementHooks";
import { SectionUtils } from "../utils/SectionUtils";
import CommitDrawer from "./CommitDrawer";
import {
    SnapshotAction,
    SnapToAction,
    useSnapshotHandler
} from "../fmActions/SnapshotAction";
import { useBlocker } from "react-router-dom";
import { ChonkyIconFA } from "chonky-icon-fontawesome";
import { useFilesFromSections } from "../hooks/BxtFsHooks";
import { CommitsState } from "../hooks/useCommits";
import { useOpenHandler } from "../fmActions/OpenAction";
import {
    CopyAction,
    CopyActionButton,
    DeleteAction,
    DeleteActionButton,
    MoveAction,
    MoveActionButton,
    useHandler
} from "../fmActions/PackageActions";
import { createCommit, mergeCommits } from "../utils/CommitUtils";

export type FileManagerProps = {
    path: string[];
    setPath: (path: string[]) => void;
    commitsState: CommitsState;
    openModalWithCommitHandler: (
        section: Section,
        commit: Commit,
        defaultCheckedAction?: ActionType
    ) => void;
    openSectionSelectModal: (cb: (section?: Section) => void) => void;
    openSnapshotModalWithBranchHandler: () => void;
    openPackageModal: () => void;
};

setChonkyDefaults({ iconComponent: ChonkyIconFA as never });

const FullFileBrowser = F as React.MemoExoticComponent<
    React.ForwardRefExoticComponent<
        FileBrowserProps & React.RefAttributes<FileBrowserHandle>
    >
>;

export default function FileManager(props: FileManagerProps) {
    const {
        path,
        setPath,
        openModalWithCommitHandler,
        openSnapshotModalWithBranchHandler,
        openPackageModal
    } = props;

    const [sections, updateSections] = useSections();

    const [drawerOpened, setDrawerOpened] = useState(false);

    const [progress, setProgress] = useState<number | undefined>(undefined);
    useBlocker(() => !!progress);
    window.onbeforeunload = () => {
        if (progress) {
            return "";
        }
    };

    const [files, updateFiles, packages] = useFilesFromSections(sections, path);

    useEffect(() => updateFiles(sections, path), [sections, path, updateFiles]);

    const snapshotHandler = useSnapshotHandler(
        openSnapshotModalWithBranchHandler
    );

    const openHandler = useOpenHandler(
        setPath,
        openPackageModal,
        packages ?? []
    );

    const { commits, addCommit, deleteCommit, clearCommits } =
        props.commitsState;

    const handleFileAction = useCallback(
        (
            actionType: "copy" | "move" | "delete",
            path: string[] | undefined
        ) => {
            if (!path) {
                return;
            }

            const [section, packageName] = [
                SectionUtils.fromPath(path),
                path[4]
            ];

            if (!section || !packageName) {
                return;
            }

            const withTargetSection = (targetSection?: Section) => {
                let action;
                switch (actionType) {
                    case "copy":
                    case "move":
                        if (!targetSection) {
                            return;
                        }
                        action = new Map<string, Section>().set(
                            packageName,
                            targetSection
                        );
                        break;
                    case "delete":
                        action = new Set<string>([packageName]);
                        break;
                }

                const existingCommit = commits.get(
                    SectionUtils.toString(section)
                );
                const commitAction = {
                    copy: "toCopy",
                    move: "toMove",
                    delete: "toDelete"
                }[actionType];

                const commit = existingCommit
                    ? mergeCommits(existingCommit, { [commitAction]: action })
                    : createCommit({ [commitAction]: action });

                openModalWithCommitHandler(section, commit, actionType);
            };

            if (actionType === "delete") {
                withTargetSection();
            } else {
                props.openSectionSelectModal(withTargetSection);
            }
        },
        [commits, openModalWithCommitHandler, props]
    );

    const copyHandler = useHandler(CopyAction.id, (path) =>
        handleFileAction("copy", path)
    );

    const moveHandler = useHandler(MoveAction.id, (path) =>
        handleFileAction("move", path)
    );

    const deleteHandler = useHandler(DeleteAction.id, (path) =>
        handleFileAction("delete", path)
    );

    const fileAction = useFileActionHandler([
        snapshotHandler,
        openHandler,
        copyHandler,
        moveHandler,
        deleteHandler
    ]);

    const folderChain = useFolderChainForPath(path);

    const fileActions = useMemo(
        () => [
            DeleteAction,
            DeleteActionButton,
            CopyAction,
            CopyActionButton,
            MoveAction,
            MoveActionButton,
            SnapToAction,
            SnapshotAction
        ],
        []
    );

    return (
        <CommitDrawer
            isOpen={drawerOpened && !progress}
            commits={commits}
            onPush={usePushCommitsHandler(commits, setProgress, () => {
                clearCommits();
                updateSections();
            })}
            onCardActivate={openModalWithCommitHandler}
            onCardDelete={(section) => {
                deleteCommit(section);
            }}
            side={true}
            onClickOverlay={() => setDrawerOpened(false)}
        >
            <Dropzone
                noClick={true}
                onDrop={usePackageDropHandler(
                    SectionUtils.fromPath(path) || sections[0],
                    (section, addAction) => {
                        const existingCommit = commits.get(
                            SectionUtils.toString(section)
                        );
                        if (!existingCommit) {
                            openModalWithCommitHandler(
                                section,
                                createCommit({ toAdd: addAction })
                            );
                            return;
                        }
                        const addCommit = mergeCommits(existingCommit, {
                            toAdd: addAction
                        });
                        openModalWithCommitHandler(section, addCommit);
                    }
                )}
            >
                {({ getRootProps, getInputProps }) => (
                    <div className="h-full" {...getRootProps()}>
                        <input {...getInputProps()} />
                        <FullFileBrowser
                            fileActions={fileActions}
                            files={files}
                            onFileAction={fileAction}
                            folderChain={folderChain}
                        />
                    </div>
                )}
            </Dropzone>
            {commits.size > 0 && !progress && (
                <Button
                    color="accent"
                    className="fixed bottom-6 right-6"
                    onClick={() => setDrawerOpened(true)}
                >
                    <FontAwesomeIcon icon={faCodeCommit} />
                    {commits.size} commit{commits.size == 1 ? "" : "s"} pending
                </Button>
            )}
            {progress && (
                <Button
                    disabled={true}
                    color="accent"
                    className="fixed bottom-6 right-6"
                >
                    <Loading />
                    Upload in progress...
                </Button>
            )}
        </CommitDrawer>
    );
}
