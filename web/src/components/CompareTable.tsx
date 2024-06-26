/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import {
    createColumnHelper,
    flexRender,
    getCoreRowModel,
    getPaginationRowModel,
    getSortedRowModel,
    useReactTable
} from "@tanstack/react-table";

import { Button, Table } from "react-daisyui";
import SectionLabel from "../components/SectionLabel";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faCodeCompare } from "@fortawesome/free-solid-svg-icons";
import { useMemo } from "react";

type CompareTableProps = {
    compareResults: CompareResult;
    resetCompareResults: () => void;
};

export default function CompareTable({
    compareResults,
    resetCompareResults
}: CompareTableProps) {
    const columnHelper = createColumnHelper<CompareEntry>();

    const columns = useMemo(() => {
        const baseColumns = [
            columnHelper.accessor("name", {
                id: "0",
                header: "Name"
            })
        ];

        if (!compareResults) {
            return baseColumns;
        }

        const sectionColumns = compareResults.sections.map((section, index) =>
            columnHelper.accessor(
                (compare: CompareEntry) => {
                    if (
                        section.branch &&
                        section.repository &&
                        section.architecture &&
                        compare[
                            `${section.branch}/${section.repository}/${section.architecture}`
                        ]
                    )
                        return Object.values(
                            compare[
                                `${section.branch}/${section.repository}/${section.architecture}`
                            ]
                        );
                },
                {
                    id: `${index + 1}`,
                    header: (props) => (
                        <SectionLabel {...props} section={section} />
                    )
                }
            )
        );

        return [...baseColumns, ...sectionColumns];
    }, [compareResults, columnHelper]);

    const data = useMemo(
        () => compareResults?.compareTable || [],
        [compareResults]
    );

    const table = useReactTable<CompareEntry>({
        columns,
        data,
        manualPagination: true,
        getCoreRowModel: getCoreRowModel(),
        getPaginationRowModel: getPaginationRowModel(),
        getSortedRowModel: getSortedRowModel()
    });

    return (
        <>
            <Table
                zebra={true}
                size="xs"
                className="w-full bg-base-100 rounded-none"
            >
                <Table.Head>
                    {table
                        .getHeaderGroups()
                        .flatMap((headerGroup) =>
                            headerGroup.headers.map((header) => (
                                <span key={header.id}>
                                    {header.isPlaceholder
                                        ? null
                                        : flexRender(
                                              header.column.columnDef.header,
                                              header.getContext()
                                          )}
                                </span>
                            ))
                        )}
                </Table.Head>
                <Table.Body>
                    {table.getRowModel().rows.map((row) => (
                        <Table.Row key={row.id}>
                            {row.getVisibleCells().map((cell) => (
                                <span key={cell.id}>
                                    {flexRender(
                                        cell.column.columnDef.cell,
                                        cell.getContext()
                                    )}
                                </span>
                            ))}
                        </Table.Row>
                    ))}
                </Table.Body>
            </Table>
            <Button
                color="accent"
                className="fixed bottom-6 right-6"
                onClick={() => resetCompareResults()}
            >
                <FontAwesomeIcon icon={faCodeCompare} />
                New compare
            </Button>
        </>
    );
}
