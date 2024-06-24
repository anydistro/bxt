/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import {
    FileArray,
    ChonkyFileActionData,
    ChonkyActions,
    FileHelper,
    FileData
} from "chonky";
import { OpenFilesPayload } from "chonky/dist/types/action-payloads.types";
import _ from "lodash";
import { useCallback } from "react";
import { SnapshotActionPayload } from "../components/SnapshotAction";

export const useFolderChainForPath = (path: string[]): FileArray => {
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

export const packageFromFilePath = (filePath: string, packages: Package[]) => {
    const parts = filePath.split("/");
    if (parts.length != 5) return;
    const section: Section = {
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
    setPackage: (pkg?: Package) => void,
    packages: Package[]
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
