/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { ChonkyIconName, FileArray, FileData } from "chonky";
import { useCallback, useEffect, useMemo, useState } from "react";
import {
    architecturesForBranchAndRepo,
    branches,
    reposForBranch
} from "../utils/SectionUtils";
import axios from "axios";

export interface IUpdateFiles {
    (sections: ISection[], path: string[]): void;
}

export const useFilesFromSections = (
    sections: ISection[],
    path: string[]
): [FileArray, IUpdateFiles, IPackage[] | undefined] => {
    const [files, setFiles] = useState<FileArray>([]);
    const [packages, setPackages] = useState<IPackage[]>();

    const getPackages = async (sections: ISection[], path: string[]) => {
        const value = await axios.get(
            `${process.env.PUBLIC_URL}/api/packages/get`,
            {
                params: {
                    branch: path[1],
                    repository: path[2],
                    architecture: path[3]
                }
            }
        );
        if (value.data == null) {
            setFiles([]);
            setPackages(undefined);
            return;
        }
        setPackages(value.data);

        setFiles(
            value.data.map(
                (value: any): FileData => ({
                    id: `root/${path[1]}/${path[2]}/${path[3]}/${value?.name}`,
                    name: value.name,
                    ext: "",
                    isDir: false,
                    thumbnailUrl:
                        value?.preferredCandidate.hasSignature == "true"
                            ? `${process.env.PUBLIC_URL}/signature.svg`
                            : `${process.env.PUBLIC_URL}/package.png`,
                    icon: ChonkyIconName.archive,
                    color: "#8B756B"
                })
            )
        );
    };

    const updateFiles = useCallback(
        (sections: ISection[], path: string[]) => {
            switch (path.length) {
                case 1:
                    setFiles(
                        branches(sections).map((value): FileData => {
                            return {
                                id: `root/${value}`,
                                name: value,
                                isDir: true,
                                thumbnailUrl: `${process.env.PUBLIC_URL}/branch.svg`,
                                color: "#8B756B"
                            };
                        })
                    );
                    break;
                case 2:
                    setFiles(
                        reposForBranch(sections, path[1]).map(
                            (value): FileData => {
                                return {
                                    id: `root/${path[1]}/${value}`,
                                    name: value,
                                    isDir: true,
                                    thumbnailUrl: `${process.env.PUBLIC_URL}/repository.png`,
                                    color: "#8B756B"
                                };
                            }
                        )
                    );
                    break;
                case 3:
                    setFiles(
                        architecturesForBranchAndRepo(
                            sections,
                            path[1],
                            path[2]
                        ).map((value): FileData => {
                            return {
                                id: `root/${path[1]}/${path[2]}/${value}`,
                                name: value,
                                isDir: true,
                                thumbnailUrl: `${process.env.PUBLIC_URL}/architecture.svg`,
                                color: "#8B756B"
                            };
                        })
                    );
                    break;
                case 4:
                    getPackages(sections, path);
                    break;
            }
        },
        [sections, path, setFiles, setPackages]
    );

    useEffect(() => {
        updateFiles(sections, path);
    }, [sections, path]);

    return [files, updateFiles, packages];
};
