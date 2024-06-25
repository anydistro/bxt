/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { faClone } from "@fortawesome/free-solid-svg-icons";
import {
    defineFileAction,
    FileData,
    thunkRequestFileAction,
    selectSelectedFiles,
    ChonkyFileActionData
} from "chonky";
import { useCallback } from "react";
import { ActionHandler } from "./ActionHandler";

export type SnapshotActionPayload = {
    sourceBranch?: string;
    targetBranch?: string;
};

export const SnapshotAction = defineFileAction({
    id: "snap",
    requiresSelection: true,
    fileFilter: (file: FileData | null) => {
        return file?.id.split("/").length == 2;
    },
    __payloadType: {} as SnapshotActionPayload
});

export const SnapToAction = defineFileAction(
    {
        id: "snap_to",
        requiresSelection: true,
        fileFilter: (file: FileData | null) => {
            return file?.id.split("/").length == 2;
        },
        button: {
            name: "Snap to...",
            contextMenu: true,
            icon: faClone
        }
    } as const,
    ({ reduxDispatch, getReduxState }) => {
        const folder = selectSelectedFiles(getReduxState());
        reduxDispatch(
            thunkRequestFileAction(SnapshotAction, {
                sourceBranch: folder[0].name
            })
        );
    }
);

export const useSnapshotHandler = (
    snapshotHandler: (sourceBranch?: string, targetBranch?: string) => void
): [string, ActionHandler] => [
    "snap",
    useCallback(
        (payload: ChonkyFileActionData) => {
            const { sourceBranch, targetBranch } =
                payload as SnapshotActionPayload;
            snapshotHandler(sourceBranch, targetBranch);
        },
        [snapshotHandler]
    )
];
