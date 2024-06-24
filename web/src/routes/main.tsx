/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import {
    FullFileBrowser as F,
    FileBrowserProps,
    FileBrowserHandle,
    setChonkyDefaults
} from "chonky";
import { ChonkyIconFA } from "chonky-icon-fontawesome";
import { useCallback, useEffect, useRef, useState } from "react";
import { usePushCommitsHandler, useSections } from "../hooks/BxtHooks";
import Dropzone from "react-dropzone-esm";
import CommitModal, { CommitModalProps } from "../modals/CommitModal";
import { Button, Loading } from "react-daisyui";
import { useFilesFromSections } from "../hooks/BxtFsHooks";
import SnapshotModal, { ISnapshotModalProps } from "../modals/SnapshotModal";
import { SnapshotAction, SnapToAction } from "../components/SnapshotAction";
import PackageModal, { PackageModalProps } from "../modals/PackageModal";
import _ from "lodash";
import CommitDrawer from "../components/CommitDrawer";
import { faCodeCommit } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    useFileActionHandler,
    useFolderChainForPath
} from "../hooks/FileManagementHooks";
import { usePackageDropHandler } from "../hooks/DragNDropHooks";
import { SectionUtils } from "../utils/SectionUtils";
import { useBlocker } from "react-router-dom";

setChonkyDefaults({ iconComponent: ChonkyIconFA as never });

const FullFileBrowser = F as React.MemoExoticComponent<
    React.ForwardRefExoticComponent<
        FileBrowserProps & React.RefAttributes<FileBrowserHandle>
    >
>;

export default function Main(props: any) {
    const [sections, updateSections] = useSections();

    const [path, setPath] = useState<string[]>(
        JSON.parse(localStorage.getItem("path") ?? '["root"]')
    );

    useEffect(() => localStorage.setItem("path", JSON.stringify(path)), [path]);

    const [files, updateFiles, packages] = useFilesFromSections(sections, path);

    useEffect(() => updateFiles(sections, path), [sections, path]);

    const [commitModalProps, setCommitModalProps] = useState<CommitModalProps>({
        isNew: true,
        section: sections[0],
        sections: sections
    });

    const openModalWithCommitHandler = (isNew: boolean) => {
        return (section: Section, commit: Commit) => {
            setIsCommitInModalNew(isNew);

            const currentCommit = commits.get(SectionUtils.toString(section));

            currentCommit?.forEach((value, key) => {
                commit.set(key, { ...value, ...commit.get(key) });
            });

            setCommitModalProps((prevCommitProps) => ({
                ...prevCommitProps,
                commit,
                section: section
            }));
            commitModalRef.current?.showModal();
        };
    };

    const [progress, setProgress] = useState<number | undefined>(undefined);
    useBlocker(() => !!progress);
    window.onbeforeunload = () => {
        if (progress) {
            return "";
        }
    };

    const commitModalRef = useRef<HTMLDialogElement>(null);

    const [commits, setCommits] = useState<Commits>(new Map());

    const [isCommitInModalNew, setIsCommitInModalNew] =
        useState<boolean>(false);

    const snapshotModalRef = useRef<HTMLDialogElement>(null);
    const packageModalRef = useRef<HTMLDialogElement>(null);

    const [drawerOpened, setDrawerOpened] = useState<boolean>(false);

    const [snapshotModalProps, setSnapshotModalProps] =
        useState<ISnapshotModalProps>({
            sections: sections
        });

    const [packageModalProps, setPackageModalProps] =
        useState<PackageModalProps>({
            package: undefined
        });

    useEffect(() => {
        setSnapshotModalProps({
            ...snapshotModalProps
        });
    }, [path]);

    useEffect(() => {
        setSnapshotModalProps({
            ...snapshotModalProps,
            sections
        });
    }, [sections]);

    const openSnapshotModalWithBranchHandler = useCallback(
        (sourceBranch?: string, targetBranch?: string) => {
            if (sourceBranch) {
                const sourceSection: Section = {
                    ...snapshotModalProps.sourceSection,
                    branch: sourceBranch
                };

                setSnapshotModalProps({
                    ...snapshotModalProps,
                    sourceSection
                });
            }
            if (targetBranch) {
                const targetSection: Section = {
                    ...snapshotModalProps.targetSection,
                    branch: targetBranch
                };

                setSnapshotModalProps({
                    ...snapshotModalProps,
                    targetSection
                });
            }

            snapshotModalRef.current?.showModal();
        },
        [snapshotModalRef, setSnapshotModalProps, snapshotModalProps]
    );

    const openPackageModal = useCallback(
        (pkg?: Package) => {
            if (!pkg) return;
            setPackageModalProps({ ...packageModalProps, package: pkg });
            packageModalRef.current?.showModal();
        },
        [packageModalRef, setPackageModalProps, packageModalProps]
    );

    return (
        <div className="flex w-full h-full items-center justify-center font-sans">
            <CommitModal
                {...commitModalProps}
                ref={commitModalRef}
                isNew={isCommitInModalNew}
                sections={sections}
                backdrop={true}
                onCommitSubmit={(section, commit) => {
                    setCommits((prevCommit) => {
                        const newCommit = new Map(prevCommit);
                        newCommit.set(SectionUtils.toString(section), commit);
                        return newCommit;
                    });

                    commitModalRef.current?.close();
                }}
                onCommitDelete={(section) => {
                    if (section) {
                        setCommits((prevCommit) => {
                            const newCommit = new Map(prevCommit);
                            newCommit.delete(SectionUtils.toString(section));
                            return newCommit;
                        });
                    }
                    commitModalRef.current?.close();
                }}
            />
            <SnapshotModal
                ref={snapshotModalRef}
                {...snapshotModalProps}
                backdrop={true}
            />
            <PackageModal
                ref={packageModalRef}
                {...packageModalProps}
                backdrop={true}
            />

            <CommitDrawer
                isOpen={drawerOpened && !progress}
                commits={commits}
                onPush={usePushCommitsHandler(commits, setProgress, () => {
                    setCommits(new Map());
                    updateSections();
                })}
                onCardActivate={openModalWithCommitHandler(true)}
                onCardDelete={(section) => {
                    setCommits((prevCommit) => {
                        const newCommit = new Map(prevCommit);
                        newCommit.delete(SectionUtils.toString(section));
                        return newCommit;
                    });
                }}
                side={true}
                onClickOverlay={() => setDrawerOpened(false)}
            >
                <Dropzone
                    noClick={true}
                    onDrop={usePackageDropHandler(
                        SectionUtils.fromPath(path),
                        openModalWithCommitHandler(false)
                    )}
                >
                    {({ getRootProps, getInputProps }) => (
                        <div className="h-full" {...getRootProps()}>
                            <input {...getInputProps()} />
                            <FullFileBrowser
                                fileActions={[SnapshotAction, SnapToAction]}
                                files={files}
                                onFileAction={useFileActionHandler(
                                    setPath,
                                    openSnapshotModalWithBranchHandler,
                                    openPackageModal,
                                    packages ?? []
                                )}
                                folderChain={useFolderChainForPath(path)}
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
                        {commits.size} commit{commits.size == 1 ? "" : "s"}{" "}
                        pending
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
        </div>
    );
}
