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

export const useSections = (): [ISection[], IUpdateSections] => {
    const [sections, setSections] = useState<ISection[]>([]);

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

export const usePackageLogs = (): [ILogEntry[], () => void] => {
    const [entries, setEntries] = useState<ILogEntry[]>([]);

    const updateEntries = useCallback(async () => {
        try {
            const result = await axios.get(
                `${process.env.PUBLIC_URL}/api/logs/packages`
            );

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
    (sections: ISection[]): void;
}

export interface IResetCompareResults {
    (): void;
}

export const useCompareResults = (): [
    ICompareResult | undefined,
    IGetCompareResults,
    IResetCompareResults
] => {
    const [results, setResults] = useState<ICompareResult>();

    const updateResults: IGetCompareResults = useCallback(
        async (sections: ISection[]) => {
            try {
                const result = await axios.post(
                    `${process.env.PUBLIC_URL}/api/compare`,
                    sections
                );

                const compareEntries: ICompareEntry[] = [];

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

export const usePushCommitsHandler = (
    commits: ICommit[],
    reload: () => void
) => {
    return useCallback(
        async (e: any) => {
            let formData = new FormData();

            let packages = commits.flatMap((value) => value.packages);
            packages.forEach((pkg, index) => {
                const missingFields = [];
                if (!pkg.file) missingFields.push("package file");
                if (!pkg.signatureFile) missingFields.push("signature file");
                if (!pkg.section.branch) missingFields.push("branch");
                if (!pkg.section.repository) missingFields.push("repository");
                if (!pkg.section.architecture)
                    missingFields.push("architecture");

                if (missingFields.length === 0) {
                    formData.append(`package${index + 1}.filepath`, pkg.file);
                    formData.append(
                        `package${index + 1}.signature`,
                        pkg.signatureFile
                    );
                    formData.append(
                        `package${index + 1}.section`,
                        JSON.stringify(pkg.section)
                    );
                } else {
                    toast.error(
                        `Missing package fields for ${
                            pkg.name
                        }: ${missingFields.join(", ")}`
                    );
                }
            });

            const result = await axios.post(
                `${process.env.PUBLIC_URL}/api/packages/commit`,
                formData
            );

            if (result.data["status"] == "ok") {
                toast.done("Pushed!");
                reload();
            }
        },
        [commits, reload]
    );
};
