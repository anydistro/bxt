/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { FullFileBrowser, setChonkyDefaults } from "chonky";
import { ChonkyIconFA } from "chonky-icon-fontawesome";
import { useCallback, useEffect, useRef, useState } from "react";
import { usePushCommitsHandler, useSections } from "../hooks/BxtHooks";
import Dropzone from "react-dropzone";
import CommitModal from "../components/CommitModal";
import { Button, Drawer, Menu } from "react-daisyui";
import CommitCard from "../components/CommitCard";
import { useFilesFromSections } from "../hooks/BxtFsHooks";
import { toast } from "react-toastify";
import SnapshotModal, {
    ISnapshotModalProps
} from "../components/SnapshotModal";
import { SnapshotAction, SnapToAction } from "../components/SnapshotAction";
import PackageModal, { PackageModalProps } from "../components/PackageModal";
import _ from "lodash";
import {
    useFileActionHandler,
    useFolderChainForPath
} from "../hooks/FileManagementHooks";
import { usePackageDropHandler } from "../hooks/DragNDropHooks";

setChonkyDefaults({ iconComponent: ChonkyIconFA });

export default (props: any) => {
    const [sections, updateSections] = useSections();

    const [path, setPath] = useState<string[]>(
        JSON.parse(localStorage.getItem("path") ?? '["root"]')
    );

    useEffect(() => localStorage.setItem("path", JSON.stringify(path)), [path]);

    const [files, updateFiles, packages] = useFilesFromSections(sections, path);

    useEffect(() => updateFiles(sections, path), [sections, path]);

    const [modalCommit, setModalCommit] = useState<ICommit>();
    const commitModalRef = useRef<HTMLDialogElement>(null);

    const [commits, setCommits] = useState<ICommit[]>([]);

    const [isCommitInModalNew, setIsCommitInModalNew] =
        useState<boolean>(false);

    const snapshotModalRef = useRef<HTMLDialogElement>(null);
    const packageModalRef = useRef<HTMLDialogElement>(null);

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

    const openModalWithCommitHandler = (isNew: boolean) => {
        return (commit: ICommit) => {
            setIsCommitInModalNew(isNew);
            setModalCommit(commit);
            commitModalRef.current?.showModal();
        };
    };

    const deleteCommitById = (id: string) => {
        toast.success("Deleted!");
        setCommits(commits.filter((value) => value.id != id));
        commitModalRef.current?.close();
    };

    const openSnapshotModalWithBranchHandler = useCallback(
        (sourceBranch?: string, targetBranch?: string) => {
            if (sourceBranch) {
                const sourceSection: ISection = {
                    ...snapshotModalProps.sourceSection,
                    branch: sourceBranch
                };

                setSnapshotModalProps({
                    ...snapshotModalProps,
                    sourceSection
                });
            }
            if (targetBranch) {
                const targetSection: ISection = {
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
        (pkg?: IPackage) => {
            if (!pkg) return;
            setPackageModalProps({ ...packageModalProps, package: pkg });
            packageModalRef.current?.showModal();
        },
        [packageModalRef, setPackageModalProps, packageModalProps]
    );

    return (
        <div className="flex w-full h-full items-center justify-center font-sans">
            <CommitModal
                ref={commitModalRef}
                isNew={isCommitInModalNew}
                sections={sections}
                backdrop={true}
                commit={modalCommit}
                onCommitSubmit={(commit) => {
                    commitModalRef.current?.close();
                    setCommits([...commits, commit]);
                }}
                onCommitDelete={deleteCommitById}
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

            <Drawer
                open={commits.length > 0}
                contentClassName="fm-content h-full"
                className="h-full"
                side={
                    <div>
                        <label
                            htmlFor="my-drawer-2"
                            className="drawer-overlay"
                        ></label>
                        <Menu className="h-screen p-4 w-100 space-y-4 bg-sky-700">
                            <li className="text-3xl font-bold text-white">
                                Pending commits
                            </li>
                            {commits?.map((value) => {
                                return (
                                    <Menu.Item>
                                        <CommitCard
                                            onActivate={openModalWithCommitHandler(
                                                true
                                            )}
                                            commit={value}
                                            onDeleteRequested={deleteCommitById}
                                        />
                                    </Menu.Item>
                                );
                            })}
                            <div className="grow"></div>
                            <Button
                                color="ghost"
                                onClick={usePushCommitsHandler(commits, () => {
                                    setCommits([]);
                                    updateSections();
                                })}
                            >
                                Push commits
                            </Button>
                        </Menu>
                    </div>
                }
                end={true}
            >
                <Dropzone
                    noClick={true}
                    onDrop={usePackageDropHandler(
                        path,
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
            </Drawer>
        </div>
    );
};
