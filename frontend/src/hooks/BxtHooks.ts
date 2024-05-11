/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { useCallback, useEffect, useState } from "react";
import axios from "axios";

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
