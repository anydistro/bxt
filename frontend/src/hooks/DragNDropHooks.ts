/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useCallback } from "react";
import * as uuid from "uuid";

export const usePackageDropHandler = (
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
                    if (!packages[name]) {
                        packages[name] = {};
                    }
                    packages[name].signatureFile = file;
                    continue;
                }

                packages[file.name] = {
                    name: file.name,
                    section,
                    file: file,
                    ...packages[file.name]
                };
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
