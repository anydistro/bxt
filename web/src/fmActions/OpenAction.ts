/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import {
    ChonkyFileActionData,
    ChonkyActions,
    FileHelper,
    FileData
} from "chonky";
import { OpenFilesPayload } from "chonky/dist/types/action-payloads.types";
import _ from "lodash";
import { useCallback } from "react";
import { ActionHandler } from "./ActionHandler";

const packageFromFilePath = (filePath: string, packages: Package[]) => {
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

export const useOpenHandler = (
    pathHandler: (path: string[]) => void,
    packageHandler: (pkg?: Package) => void,
    packages: Package[]
): [string, ActionHandler] => [
    ChonkyActions.OpenFiles.id,
    useCallback(
        (data: ChonkyFileActionData) => {
            const { targetFile, files } = data.payload as OpenFilesPayload;
            const fileToOpen = targetFile ?? files[0];
            if (fileToOpen && FileHelper.isDirectory(fileToOpen)) {
                const pathToOpen = fileToOpen.id.split("/");
                pathHandler(pathToOpen);
            } else if (fileToOpen && !FileHelper.isDirectory(fileToOpen)) {
                packageHandler(
                    packageFromFilePath((fileToOpen as FileData).id, packages)
                );
            }
        },
        [pathHandler, packageHandler, packages]
    )
];
