/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useEffect, useState } from "react";
import { faCodeCommit } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    FileArray,
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

export type FileManagerProps = {
    path: string[];
    setPath: (path: string[]) => void;
    commitsState: CommitsState;
    openModalWithCommitHandler: (
        isNew: boolean
    ) => (section: Section, commit: Commit) => void;
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

    const fileAction = useFileActionHandler([snapshotHandler, openHandler]);

    const { commits, addCommit, deleteCommit, clearCommits } =
        props.commitsState;

    const folderChain = useFolderChainForPath(path);

    return (
        <CommitDrawer
            isOpen={drawerOpened && !progress}
            commits={commits}
            onPush={usePushCommitsHandler(commits, setProgress, () => {
                clearCommits();
                updateSections();
            })}
            onCardActivate={openModalWithCommitHandler(true)}
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
                    openModalWithCommitHandler(false)
                )}
            >
                {({ getRootProps, getInputProps }) => (
                    <div className="h-full" {...getRootProps()}>
                        <input {...getInputProps()} />
                        <FullFileBrowser
                            fileActions={[SnapToAction, SnapshotAction]}
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
