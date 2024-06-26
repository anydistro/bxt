/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import React from "react";
import { Loading, Card } from "react-daisyui";
import { usePackageLogs } from "../hooks/BxtHooks";
import { SortingState } from "@tanstack/react-table";
import LogTable from "../components/LogTable";
import { faBan } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";

export default function Log(props: any) {
    const [entries, updateEntries, isLoading] = usePackageLogs();
    const [sorting, setSorting] = React.useState<SortingState>([
        { id: "Time", desc: true }
    ]);
    const renderContent = () => {
        if (isLoading) {
            return (
                <div className="grow flex w-full justify-center content-center">
                    <Loading variant="bars" className="w-20" />
                </div>
            );
        }

        if (!entries) {
            return (
                <div className="overflow-y-auto flex h-full w-full justify-center items-center">
                    <Card
                        className="m-auto bg-base-100 shadow-sm"
                        compact={true}
                    >
                        <Card.Body>
                            <Card.Title>
                                <FontAwesomeIcon icon={faBan} />
                                Not found
                            </Card.Title>
                            No events was found. It may happen because there
                            were no actions yet or the database is corrupted.
                        </Card.Body>
                    </Card>
                </div>
            );
        }

        return (
            <LogTable
                entries={entries!}
                sorting={sorting}
                setSorting={setSorting}
            />
        );
    };

    return (
        <div className="flex flex-col w-full h-full overflow-x-auto bg-base-200">
            {renderContent()}
        </div>
    );
}
