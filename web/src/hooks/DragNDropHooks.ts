/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useCallback } from "react";

export const usePackageDropHandler = (
    section: Section | undefined,
    add: (section: Section, commit: AddAction) => void
) => {
    return useCallback(
        (acceptedFiles: File[]) => {
            if (!section) return;
            const packages: {
                [packageName: string]: Partial<PackageUpload>;
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

            add(section, new Map(Object.entries(packages)));
        },
        [section, add]
    );
};
