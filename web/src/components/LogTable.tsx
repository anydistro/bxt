/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import React from "react";
import { Table } from "react-daisyui";
import {
    useReactTable,
    flexRender,
    getCoreRowModel,
    getPaginationRowModel,
    getSortedRowModel,
    SortingState,
    createColumnHelper
} from "@tanstack/react-table";
import SectionLabel from "../components/SectionLabel";

interface LogTableProps {
    entries: LogEntry[];
    sorting: SortingState;
    setSorting: React.Dispatch<React.SetStateAction<SortingState>>;
}

const LogTable = ({ entries, sorting, setSorting }: LogTableProps) => {
    const columnHelper = createColumnHelper<LogEntry>();

    const columns = [
        columnHelper.accessor("type", {
            header: "Type"
        }),
        columnHelper.accessor("package.name", {
            header: "Name"
        }),

        columnHelper.accessor("package.poolEntries", {
            header: "Version",
            cell: (context) => {
                const value = context.getValue();
                const preferredLocation =
                    context?.row?.original?.package?.preferredLocation;

                if (value && preferredLocation && value[preferredLocation]) {
                    return value[preferredLocation].version;
                } else {
                    return "Unknown Version";
                }
            }
        }),

        columnHelper.accessor("package.section", {
            header: "Section",
            cell: (context) => <SectionLabel section={context.getValue()} />
        }),

        columnHelper.accessor((entry) => entry.time, {
            header: "Time",
            enableSorting: true,
            sortingFn: "datetime"
        })
    ];

    const table = useReactTable<LogEntry>({
        columns,
        data: entries,
        state: {
            sorting
        },
        manualPagination: true,
        sortDescFirst: true,
        enableSorting: true,
        onSortingChange: setSorting,
        getCoreRowModel: getCoreRowModel(),
        getPaginationRowModel: getPaginationRowModel(),
        getSortedRowModel: getSortedRowModel(),
        initialState: {}
    });

    return (
        <Table size="xs" zebra={true} className="w-full">
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
                {table.getRowModel().rows.map((row) => {
                    return (
                        <Table.Row key={row.id}>
                            {row.getVisibleCells().map((cell) => {
                                return (
                                    <span key={cell.id}>
                                        {flexRender(
                                            cell.column.columnDef.cell,
                                            cell.getContext()
                                        )}
                                    </span>
                                );
                            })}
                        </Table.Row>
                    );
                })}
            </Table.Body>
        </Table>
    );
};

export default LogTable;
