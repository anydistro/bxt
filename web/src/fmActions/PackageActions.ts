/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import {
    faCopy,
    faTrashAlt,
    faArrowsAlt,
    IconDefinition
} from "@fortawesome/free-solid-svg-icons";
import {
    ChonkyFileActionData,
    defineFileAction,
    FileAction,
    FileData,
    selectSelectedFiles,
    thunkRequestFileAction
} from "chonky";
import { useCallback } from "react";
import { ActionHandler } from "./ActionHandler";

export type ActionPayload = {
    path?: string[];
};

const definePayloadAction = (actionId: string) => {
    return defineFileAction({
        id: actionId,
        requiresSelection: true,
        fileFilter: (file: FileData | null) => file?.id.split("/").length == 5,
        __payloadType: {} as ActionPayload
    } as const);
};

const defineButtonAction = (
    action: FileAction,
    name: string,
    icon: IconDefinition
) => {
    return defineFileAction(
        {
            id: action.id + "Button",
            requiresSelection: true,
            fileFilter: (file: FileData | null) =>
                file?.id.split("/").length == 5,
            button: {
                name,
                contextMenu: true,
                icon
            }
        } as const,
        ({ reduxDispatch, getReduxState }) => {
            const folder = selectSelectedFiles(getReduxState());
            reduxDispatch(
                thunkRequestFileAction(action, {
                    path: folder[0].id.split("/")
                })
            );
        }
    );
};

export const MoveAction = definePayloadAction("move");

export const MoveActionButton = defineButtonAction(
    MoveAction,
    "Move",
    faArrowsAlt
);
export const CopyAction = definePayloadAction("copy");

export const CopyActionButton = defineButtonAction(CopyAction, "Copy", faCopy);

export const DeleteAction = definePayloadAction("delete");

export const DeleteActionButton = defineButtonAction(
    DeleteAction,
    "Delete",
    faTrashAlt
);

export const useHandler = (
    actionId: string,
    handler: (path: string[]) => void
): [string, ActionHandler] => [
    actionId,
    useCallback(
        ({ payload }: ChonkyFileActionData) => {
            const { path } = payload as ActionPayload;
            if (!path) {
                return;
            }

            handler(path);
        },
        [handler]
    )
];
