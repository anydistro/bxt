import { useCallback, useEffect, useState } from "react";
import packages from "./packages.json";
import axios from "axios";
import { toast } from "react-toastify";

export interface IUpdateSections {
    (): void;
}

export const useSections = (): [ISection[], IUpdateSections] => {
    const [sections, setSections] = useState<ISection[]>([]);

    const updateSections: IUpdateSections = useCallback(() => {
        axios
            .get(`${process.env.PUBLIC_URL}/api/sections/get`)
            .then((response) => {
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
        } catch (error) {
            setEntries([]);
        }
    }, [setEntries]);

    return [entries, updateEntries];
};

export interface IGetCompareResults {
    (sections: ISection[]): void;
}

export const useCompareResults = (): [
    ICompareResult | undefined,
    IGetCompareResults
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

                Object.keys(result.data["compare_table"]).forEach((value) => {
                    const versions = { ...result.data["compare_table"] }[value];

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

    return [results, updateResults];
};
