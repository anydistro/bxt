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
import { mergeCommits } from "../utils/CommitUtils";
import SectionSelectModal, {
    SectionSelectModalProps
} from "../modals/SectionSelectModal";
import useLocalStorage from "../hooks/useLocalStorage";

const useLocalStoredCommits = () => {
    const reviver = (key: string, value: any) => {
        if (value.dataType === "Map") {
            return new Map(value.value);
        }
        if (value.dataType === "Set") {
            return new Set(value.value);
        }
        return value;
    };

    const replacer = (key: string, value: any) => {
        if (value instanceof Map) {
            return {
                dataType: "Map",
                value: [...value.entries()]
            };
        }
        if (value instanceof Set) {
            return {
                dataType: "Set",
                value: [...value.entries()]
            };
        }
        return value;
    };

    const commitsState = useCommits(() =>
        localStorage.getItem("commits")
            ? new Map(
                  JSON.parse(localStorage.getItem("commits") || "", reviver)
              )
            : new Map()
    );

    const { commits } = commitsState;

    useEffect(() => {
        localStorage.setItem("commits", JSON.stringify(commits, replacer));
    }, [commits]);

    return commitsState;
};

export default function Main(props: any) {
    const [sections, updateSections] = useSections();

    const [path, setPath] = useLocalStorage<string[]>("path", ["root"]);

    const [selection, setSelection] = useLocalStorage<string[]>(
        "selection",
        []
    );

    const [commitModalProps, setCommitModalProps] = useState<CommitModalProps>({
        isNew: true,
        section: sections[0],
        sections: sections
    });
    const commitModalRef = useRef<HTMLDialogElement>(null);

    const commitsState = useLocalStoredCommits();
    const { commits, addCommit, deleteCommit, clearCommits } = commitsState;

    const [isCommitInModalNew, setIsCommitInModalNew] =
        useState<boolean>(false);

    const snapshotModalRef = useRef<HTMLDialogElement>(null);
    const packageModalRef = useRef<HTMLDialogElement>(null);
    const sectionSelectModalRef = useRef<HTMLDialogElement>(null);

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

    const openModalWithCommitHandler = useCallback(
        (
            section: Section,
            commit: Commit,
            defaultCheckedAction?: ActionType
        ) => {
            const existingCommit = commits.get(SectionUtils.toString(section));
            if (!existingCommit) {
                setIsCommitInModalNew(true);
                setCommitModalProps((prevCommitProps) => ({
                    ...prevCommitProps,
                    commit: commit,
                    section: section,
                    defaultCheckedAction: defaultCheckedAction
                }));

                commitModalRef.current?.showModal();
                return;
            }

            setIsCommitInModalNew(false);

            setCommitModalProps((prevCommitProps) => ({
                ...prevCommitProps,
                commit: mergeCommits(existingCommit, commit),
                section: section,
                defaultCheckedAction: defaultCheckedAction
            }));
            commitModalRef.current?.showModal();
        },
        [commitModalRef, setCommitModalProps, commits]
    );
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

    const [sectionSelectModalProps, setSectionSelectModalProps] =
        useState<SectionSelectModalProps>({
            sections: sections
        });

    const openSectionSelectModal = useCallback(
        (cb: (section?: Section) => void) => {
            setSectionSelectModalProps({
                ...sectionSelectModalProps,
                onSectionChange: (section) => {
                    console.log("section", section);
                    sectionSelectModalRef.current?.close();
                    cb(section);
                }
            });

            sectionSelectModalRef.current?.showModal();
        },
        [
            sectionSelectModalRef,
            setSectionSelectModalProps,
            sectionSelectModalProps
        ]
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
            <SectionSelectModal
                ref={sectionSelectModalRef}
                {...sectionSelectModalProps}
                sections={sections}
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
                openSectionSelectModal={openSectionSelectModal}
                openPackageModal={openPackageModal}
            />
        </div>
    );
}
