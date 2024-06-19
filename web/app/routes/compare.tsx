/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useState, useEffect, useMemo } from "react";
import {
    createColumnHelper,
    flexRender,
    getCoreRowModel,
    getPaginationRowModel,
    getSortedRowModel,
    useReactTable
} from "@tanstack/react-table";
import { useCompareResults, useSections } from "../hooks/BxtHooks";
import { Button, Card, Loading, Table } from "react-daisyui";
import SectionSelect from "../components/SectionSelect";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    faCodeCompare,
    faPlus,
    faTrash
} from "@fortawesome/free-solid-svg-icons";
import SectionLabel from "../components/SectionLabel";

export default (props: any) => {
    const [sections, updateSections] = useSections();
    const [selectedSections, setSelectedSections] = useState<ISection[]>([{}]);
    const [compareResults, getCompareResults, resetCompareResults] =
        useCompareResults();

    const [isLoading, setIsLoading] = useState<boolean>(false);

    const updateSectionAt = (index: number, update: any) => {
        setSelectedSections((prevSections) => [
            ...prevSections.slice(0, index),
            { ...sections[0], ...prevSections[index], ...update },
            ...prevSections.slice(index + 1)
        ]);
    };

    useEffect(() => {
        setIsLoading(false);
    }, [compareResults]);

    const columnHelper = createColumnHelper<ICompareEntry>();

    const columns = [
        columnHelper.accessor("name", {
            id: "0",
            header: "Name"
        }),
        ...(compareResults?.sections.map((section, index) =>
            columnHelper.accessor(
                (compare: ICompareEntry) => {
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
        ) || [])
    ];

    const data = useMemo(
        () => compareResults?.compareTable || [],
        [compareResults]
    );

    const table = useReactTable<ICompareEntry>({
        columns,
        data,
        manualPagination: true,
        getCoreRowModel: getCoreRowModel(),
        getPaginationRowModel: getPaginationRowModel(),
        getSortedRowModel: getSortedRowModel()
    });

    return (
        <div className="w-full h-full bg-base-200 overflow-x-auto">
            {compareResults ? (
                isLoading ? (
                    <div className="grow flex w-full justify-center content-center">
                        <Loading variant="bars" className="w-20" />
                    </div>
                ) : (
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
                                                      header.column.columnDef
                                                          .header,
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
                        <Button
                            color="accent"
                            className="fixed bottom-6 right-6"
                            onClick={() => resetCompareResults()}
                        >
                            <FontAwesomeIcon icon={faCodeCompare} />
                            New compare
                        </Button>
                    </Table>
                )
            ) : (
                <div className="overflow-y-auto flex h-full w-full justify-center items-center">
                    <Card
                        className="m-auto bg-base-100 shadow-sm"
                        compact={true}
                    >
                        <Card.Body>
                            <Card.Title>Compare sections</Card.Title>
                            Choose sections you wish to compare.
                            <span className="pt-2 px-10 space-y-4 flex-col justify-center">
                                {selectedSections.map((section, index) => (
                                    <div
                                        className="flex items-end h-fit -mr-14"
                                        key={index}
                                    >
                                        <SectionSelect
                                            sections={sections}
                                            selectedSection={section}
                                            onSelected={(section) =>
                                                updateSectionAt(index, section)
                                            }
                                        />

                                        <Button
                                            className={`mx-2 ${
                                                index + 1 ==
                                                    selectedSections.length &&
                                                selectedSections.length > 1
                                                    ? ""
                                                    : "invisible"
                                            }`}
                                            size="sm"
                                            color="ghost"
                                            onClick={() =>
                                                setSelectedSections([
                                                    ...selectedSections.slice(
                                                        0,
                                                        index
                                                    ),
                                                    ...selectedSections.slice(
                                                        index + 1
                                                    )
                                                ])
                                            }
                                        >
                                            <FontAwesomeIcon icon={faTrash} />
                                        </Button>
                                    </div>
                                ))}
                                <Button
                                    className="w-full"
                                    color="ghost"
                                    size="sm"
                                    onClick={() =>
                                        setSelectedSections([
                                            ...selectedSections,
                                            {}
                                        ])
                                    }
                                >
                                    <FontAwesomeIcon
                                        className="mr-2"
                                        icon={faPlus}
                                    />
                                    Add section
                                </Button>
                            </span>
                            <Card.Actions className="justify-end">
                                <Button
                                    size="sm"
                                    color="accent"
                                    onClick={() => {
                                        setIsLoading(true);
                                        getCompareResults(selectedSections);
                                    }}
                                >
                                    <FontAwesomeIcon
                                        className="mr-2"
                                        icon={faCodeCompare}
                                    />
                                    Compare
                                </Button>
                            </Card.Actions>
                        </Card.Body>
                    </Card>
                </div>
            )}
        </div>
    );
};
