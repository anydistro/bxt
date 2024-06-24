/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { useCallback, useEffect, useState } from "react";
import axios from "axios";
import { toast } from "react-toastify";

export interface IUpdateSections {
    (): void;
}

export const useSections = (): [Section[], IUpdateSections] => {
    const [sections, setSections] = useState<Section[]>([]);

    const updateSections: IUpdateSections = useCallback(() => {
        axios.get(`/api/sections`).then((response) => {
            setSections(response.data);
        });
    }, [setSections]);

    useEffect(() => {
        updateSections();
    }, []);

    return [sections, updateSections];
};

export const usePackageLogs = (): [LogEntry[], () => void] => {
    const [entries, setEntries] = useState<LogEntry[]>([]);

    const updateEntries = useCallback(async () => {
        try {
            const result = await axios.get(`/api/logs/packages`);

            const entries = result.data.map((value: any) => {
                value.time = new Date(value.time);
                return value;
            });
            setEntries(entries);
        } catch (error) {}
    }, [setEntries]);

    return [entries, updateEntries];
};

export interface IGetCompareResults {
    (sections: Section[]): void;
}

export interface IResetCompareResults {
    (): void;
}

export const useCompareResults = (): [
    CompareResult | undefined,
    IGetCompareResults,
    IResetCompareResults
] => {
    const [results, setResults] = useState<CompareResult>();

    const updateResults: IGetCompareResults = useCallback(
        async (sections: Section[]) => {
            try {
                const result = await axios.post(`/api/compare`, sections);

                const compareEntries: CompareEntry[] = [];

                Object.keys(result.data["compareTable"]).forEach((value) => {
                    const versions = { ...result.data["compareTable"] }[value];

                    compareEntries.push({ name: value, ...versions });
                });

                setResults({
                    sections: result.data.sections,
                    compareTable: compareEntries
                });
            } catch (error) {
                setResults({ sections: [], compareTable: [] });
            }
        },
        [setResults]
    );

    return [results, updateResults, () => setResults(undefined)];
};

const formFromCommits = (commits: Commits) => {
    let index = 0;

    let formData = new FormData();
    for (const [_, commit] of Array.from(commits)) {
        for (const [name, pkg] of Array.from(commit)) {
            const missingFields = [];
            if (!pkg.file) missingFields.push("package file");
            if (!pkg.signatureFile) missingFields.push("signature file");
            if (!pkg.section) missingFields.push("section");
            if (!pkg.section?.branch) missingFields.push("branch");
            if (!pkg.section?.repository) missingFields.push("repository");
            if (!pkg.section?.architecture) missingFields.push("architecture");

            if (missingFields.length === 0) {
                formData.append(`package${index + 1}.filepath`, pkg.file!);
                formData.append(
                    `package${index + 1}.signature`,
                    pkg.signatureFile!
                );
                formData.append(
                    `package${index + 1}.section`,
                    JSON.stringify(pkg.section)
                );
            } else {
                return {
                    form: formData,
                    missingFields: missingFields,
                    errorPackage: name
                };
            }
            index++;
        }
    }
    return {
        form: formData,
        missingFields: undefined,
        errorPackage: undefined
    };
};

export const usePushCommitsHandler = (
    commits: Commits,
    onProgress: (progress: number | undefined) => void,
    reload: () => void
) => {
    return useCallback(async () => {
        const { form, missingFields, errorPackage } = formFromCommits(commits);

        if (missingFields) {
            toast.error(
                `Missing fields for package ${errorPackage}: ${missingFields.join(
                    ", "
                )}`
            );
            return;
        }

        const result = await axios.post(`/api/packages/commit`, form, {
            onUploadProgress: (p) => {
                const progress = p.loaded / (p.total || 1);
                onProgress(progress);
            }
        });

        if (result.data["status"] == "ok") {
            onProgress(undefined);
            reload();
        }
    }, [commits, reload, onProgress]);
};
