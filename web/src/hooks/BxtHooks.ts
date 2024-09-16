/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { useCallback, useEffect, useState } from "react";
import axios from "axios";
import { toast } from "react-toastify";
import { Log } from "../definitions/log";
import { addAxiosDateTransformer } from "axios-date-transformer";
import { SectionUtils } from "../utils/SectionUtils";

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

export type IUpdateLogs = (since: Date, until: Date, fullText?: string) => void;

export const useLogs = (
    initualSince: Date,
    initualUntil: Date
): [Log | undefined, IUpdateLogs, boolean] => {
    const [logs, setLogs] = useState<Log>();
    const [isLoading, setIsLoading] = useState<boolean>(false);

    const updateLogs: IUpdateLogs = useCallback(
        async (since, until, fullText) => {
            setIsLoading(true);
            const response = await addAxiosDateTransformer(axios).get<Log>(
                `/api/logs`,
                {
                    params: {
                        since: since.toISOString(),
                        until: until.toISOString(),
                        text: fullText
                    }
                }
            );

            setLogs(response.data);
            setIsLoading(false);
        },
        [setLogs]
    );

    useEffect(() => {
        updateLogs(initualSince, initualUntil);
    }, [updateLogs]);

    return [logs, updateLogs, isLoading];
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

    const formData = new FormData();
    for (const [_, commit] of Array.from(commits)) {
        for (const [name, pkg] of Array.from(commit.toAdd)) {
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

    formData.append(
        "to_delete",
        JSON.stringify(
            Array.from(commits.entries())
                .map(([section, commit]) =>
                    Array.from(commit.toDelete.entries()).map(([name, _]) => ({
                        name,
                        section: SectionUtils.fromString(section)
                    }))
                )
                .flat()
        )
    );

    formData.append(
        "to_copy",
        JSON.stringify(
            Array.from(commits.entries())
                .map(([section, commit]) =>
                    Array.from(commit.toCopy.entries()).map(
                        ([name, targetSection]) => ({
                            name,
                            from_section: SectionUtils.fromString(section),
                            to_section: targetSection
                        })
                    )
                )
                .flat()
        )
    );

    formData.append(
        "to_move",
        JSON.stringify(
            Array.from(commits.entries())
                .map(([section, commit]) =>
                    Array.from(commit.toMove.entries()).map(
                        ([name, targetSection]) => ({
                            name,
                            from_section: SectionUtils.fromString(section),
                            to_section: targetSection
                        })
                    )
                )
                .flat()
        )
    );

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
        try {
            // We can't allow requests without a valid token since the backend
            // processes all files before responding. An invalid token would
            // cause a full package upload resulting in an error and requiring
            // the entire process to be repeated.
            await axios.get("/api/auth/refresh");

            const result = await axios.post(`/api/packages/commit`, form, {
                onUploadProgress: (p) => {
                    const progress = p.loaded / (p.total || 1);
                    onProgress(progress);
                }
            });
            onProgress(undefined);

            if (result.data["status"] == "ok") {
                reload();
            }
        } catch {
            onProgress(undefined);
        }
    }, [commits, reload, onProgress]);
};
