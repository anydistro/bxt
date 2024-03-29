/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import React, { useEffect, useState } from "react";
import { Table, Loading } from "react-daisyui";
import { usePackageLogs } from "../hooks/BxtHooks";
import {
    useReactTable,
    createColumnHelper,
    getCoreRowModel,
    getPaginationRowModel,
    getSortedRowModel,
    flexRender,
    SortingState
} from "@tanstack/react-table";
import SectionLabel from "../components/SectionLabel";

const dateCompare = (a: ILogEntry, b: ILogEntry) => {
    if (a.time < b.time) {
        return -1;
    }
    if (a.time > b.time) {
        return 1;
    }
    return 0;
};

export default (props: any) => {
    const [entries, updateEntries] = usePackageLogs();
    const [isLoading, setIsLoading] = useState(false);
    const [sorting, setSorting] = React.useState<SortingState>([
        { id: "Time", desc: true }
    ]);

    useEffect(() => {
        setIsLoading(false);
    }, [entries]);

    useEffect(() => {
        updateEntries();
        setIsLoading(true);
    }, []);

    const columnHelper = createColumnHelper<ILogEntry>();

    const columns = [
        columnHelper.accessor("action", {
            header: "Action"
        }),
        columnHelper.accessor("package.name", {
            header: "Name"
        }),

        columnHelper.accessor("package.preferredCandidate.version", {
            header: "Version"
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

    const table = useReactTable<ILogEntry>({
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
        <div className="flex flex-col w-full h-full overflow-x-auto">
            {isLoading ? (
                <div className="grow flex w-full justify-center content-center">
                    <Loading variant="bars" className="w-20" />
                </div>
            ) : (
                <Table size="xs" zebra={true} className="w-full" {...props}>
                    <Table.Head>
                        {table
                            .getHeaderGroups()
                            .flatMap((headerGroup) =>
                                headerGroup.headers.map((header) => (
                                    <span key={header.id}>
                                        {header.isPlaceholder
                                            ? null
                                            : flexRender(
                                                  header.column.columnDef
                                                      .header,
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
            )}
        </div>
    );
};
