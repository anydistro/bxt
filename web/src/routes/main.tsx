/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useCallback, useEffect, useRef, useState } from "react";
import { useSections } from "../hooks/BxtHooks";
import CommitModal, { CommitModalProps } from "../modals/CommitModal";
import SnapshotModal, { ISnapshotModalProps } from "../modals/SnapshotModal";
import PackageModal, { PackageModalProps } from "../modals/PackageModal";
import { SectionUtils } from "../utils/SectionUtils";
import FileManager from "../components/FileManager";
import useCommits from "../hooks/useCommits";

export default function Main(props: any) {
    const [sections, updateSections] = useSections();

    const [path, setPath] = useState<string[]>(() => {
        const storedPath = localStorage.getItem("path");
        return storedPath ? JSON.parse(storedPath) : ["root"];
    });

    useEffect(() => {
        localStorage.setItem("path", JSON.stringify(path));
    }, [path]);

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

    const commitModalRef = useRef<HTMLDialogElement>(null);

    const commitsState = useCommits();

    const { commits, addCommit, deleteCommit, clearCommits } = commitsState;

    const [isCommitInModalNew, setIsCommitInModalNew] =
        useState<boolean>(false);

    const snapshotModalRef = useRef<HTMLDialogElement>(null);
    const packageModalRef = useRef<HTMLDialogElement>(null);

    const [snapshotModalProps, setSnapshotModalProps] =
        useState<ISnapshotModalProps>({
            sections: sections
        });

    const [packageModalProps, setPackageModalProps] =
        useState<PackageModalProps>({});

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
                    addCommit(section, commit);
                    commitModalRef.current?.close();
                }}
                onCommitDelete={(section) => {
                    if (section) {
                        deleteCommit(section);
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
            <FileManager
                path={path}
                setPath={setPath}
                commitsState={commitsState}
                openModalWithCommitHandler={openModalWithCommitHandler}
                openSnapshotModalWithBranchHandler={
                    openSnapshotModalWithBranchHandler
                }
                openPackageModal={openPackageModal}
            />
        </div>
    );
}
