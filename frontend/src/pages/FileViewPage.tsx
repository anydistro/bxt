/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import {
    FullFileBrowser,
    FileArray,
    setChonkyDefaults,
    ChonkyFileActionData,
    ChonkyActions,
    FileHelper,
    FileData
} from "chonky";
import { ChonkyIconFA } from "chonky-icon-fontawesome";
import { useCallback, useEffect, useRef, useState } from "react";
import { useSections } from "../hooks/BxtHooks";
import Dropzone from "react-dropzone";
import CommitModal from "../components/CommitModal";
import { Button, Drawer, Menu } from "react-daisyui";
import CommitCard from "../components/CommitCard";
import { useFilesFromSections } from "../hooks/BxtFsHooks";
import * as uuid from "uuid";
import { toast } from "react-toastify";
import SnapshotModal, {
    ISnapshotModalProps
} from "../components/SnapshotModal";
import {
    SnapshotAction,
    SnapshotActionPayload,
    SnapToAction
} from "../components/SnapshotAction";
import { OpenFilesPayload } from "chonky/dist/types/action-payloads.types";
import axios from "axios";
import PackageModal, { PackageModalProps } from "../components/PackageModal";
import _ from "lodash";

setChonkyDefaults({ iconComponent: ChonkyIconFA });

const useFolderChainForPath = (path: string[]): FileArray => {
    const result: FileArray = [];

    result.push({
        id: "root",
        name: "root",
        isDir: true
    });

    for (let i = 1; i < path.length; i++) {
        result.push({
            id: `${result[i - 1]!.id}/${path[i]}`,
            name: path[i],
            isDir: true
        });
    }

    return result;
};

const packageFromFilePath = (filePath: string, packages: IPackage[]) => {
    const parts = filePath.split("/");
    if (parts.length != 5) return;
    const section: ISection = {
        branch: parts[1],
        repository: parts[2],
        architecture: parts[3]
    };
    const packageName = parts[4];

    return packages.find((value) => {
        return _.isEqual(value.section, section) && value.name == packageName;
    });
};

export const useFileActionHandler = (
    setPath: (path: string[]) => void,
    setSnapshotModalBranches: (
        sourceBranch?: string,
        targetBranch?: string
    ) => void,
    setPackage: (pkg?: IPackage) => void,
    packages: IPackage[]
) => {
    return useCallback(
        (data: ChonkyFileActionData) => {
            switch (data.id as string) {
                case ChonkyActions.OpenFiles.id:
                    const { targetFile, files } =
                        data.payload as OpenFilesPayload;
                    const fileToOpen = targetFile ?? files[0];
                    if (fileToOpen && FileHelper.isDirectory(fileToOpen)) {
                        const pathToOpen = fileToOpen.id.split("/");

                        setPath(pathToOpen);
                    } else if (
                        fileToOpen &&
                        !FileHelper.isDirectory(fileToOpen)
                    ) {
                        setPackage(
                            packageFromFilePath(
                                (fileToOpen as FileData).id,
                                packages
                            )
                        );
                    }
                    break;
                case "snap":
                    const { sourceBranch, targetBranch } =
                        data.payload as SnapshotActionPayload;

                    setSnapshotModalBranches(sourceBranch, targetBranch);
            }
        },
        [setPath, setSnapshotModalBranches, setPackage, packages]
    );
};

const usePackageDropHandler = (
    path: string[],
    setCommit: (commits: ICommit) => void
) => {
    return useCallback(
        (acceptedFiles: File[]) => {
            const section: ISection = {
                branch: path[1],
                repository: path[2],
                architecture: path[3]
            };
            const packages: {
                [key: string]: Partial<IPackageUpload>;
            } = {};
            for (const file of acceptedFiles) {
                if (file.name.endsWith(".sig")) {
                    const name = file.name.replace(".sig", "");

                    packages[name].signatureFile = file;
                    continue;
                }

                if (!packages[file.name]) {
                    packages[file.name] = {
                        name: file.name,
                        signatureFile: file.name.endsWith(".sig")
                            ? file
                            : undefined,
                        section,
                        file: file
                    };
                }
            }

            setCommit({
                id: uuid.v4(),
                section,
                packages: Object.values(packages)
                    .filter((partial) => partial as IPackageUpload)
                    .map((partial) => partial as IPackageUpload)
            });
        },
        [path, setCommit]
    );
};

const usePushCommitsHandler = (commits: ICommit[], reload: () => void) => {
    return useCallback(
        async (e: any) => {
            let formData = new FormData();

            let packages = commits.flatMap((value) => value.packages);
            packages.forEach((pkg, index) => {
                const missingFields = [];
                if (!pkg.file) missingFields.push("package file");
                if (!pkg.signatureFile) missingFields.push("signature file");
                if (!pkg.section.branch) missingFields.push("branch");
                if (!pkg.section.repository) missingFields.push("repository");
                if (!pkg.section.architecture)
                    missingFields.push("architecture");

                if (missingFields.length === 0) {
                    formData.append(`package${index + 1}.filepath`, pkg.file);
                    formData.append(
                        `package${index + 1}.signature`,
                        pkg.signatureFile
                    );
                    formData.append(
                        `package${index + 1}.section`,
                        JSON.stringify(pkg.section)
                    );
                } else {
                    toast.error(
                        `Missing package fields for ${
                            pkg.name
                        }: ${missingFields.join(", ")}`
                    );
                }
            });

            const result = await axios.post(
                `${process.env.PUBLIC_URL}/api/packages/commit`,
                formData
            );

            if (result.data["status"] == "ok") {
                toast.done("Pushed!");
                reload();
            }
        },
        [commits, reload]
    );
};

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
