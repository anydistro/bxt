/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { FileArray, ChonkyFileActionData } from "chonky";
import _ from "lodash";
import { useCallback, useEffect, useState } from "react";
import { ActionHandlers } from "../fmActions/ActionHandler";

export const useFolderChainForPath = (path: string[]): FileArray => {
    const [result, setResult] = useState<FileArray>([
        {
            id: "root",
            name: "root",
            isDir: true
        }
    ]);

    useEffect(() => {
        const newResult = [
            {
                id: "root",
                name: "root",
                isDir: true
            }
        ];
        for (let i = 1; i < path.length; i++) {
            newResult.push({
                id: `${newResult[i - 1]!.id}/${path[i]}`,
                name: path[i],
                isDir: true
            });
        }
        setResult(newResult);
    }, [path, setResult]);

    return result;
};

export const useFileActionHandler = (actionHandlers: ActionHandlers) => {
    return useCallback(
        (data: ChonkyFileActionData) => {
            const handler = actionHandlers.find(
                (value) => value[0] === data.id
            )?.[1];
            if (handler) {
                handler(data);
            }
        },
        [actionHandlers]
    );
};
