/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { useEffect, useMemo, useRef, useState } from "react";
import {
    Loading,
    Card,
    Divider,
    Button,
    Pagination,
    Checkbox
} from "react-daisyui";
import { useLogs } from "../hooks/BxtHooks";
import LogTable from "../components/LogTable";
import {
    faArrowsRotate,
    faBan,
    faClock,
    faCodeCommit,
    faCubes,
    faList,
    faRocket,
    faSearch
} from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { subDays } from "date-fns";
import DateRangeInput from "../components/DateRangeInput";
import _ from "lodash";
import { LogEntry } from "../definitions/log";
import LogEntryModal from "../modals/LogEntryModal";

const entriesPerPage = 20;

export default function Log(props: any) {
    const [dateRange, setDateRange] = useState({
        since: new Date(subDays(new Date(), 7).getTime()),
        until: new Date()
    });

    const [entries, updateEntries, isLoading] = useLogs(
        dateRange.since,
        dateRange.until
    );

    const [selectedPage, setSelectedPage] = useState(1);
    const [numOfPages, setNumOfPages] = useState(1);

    const [showEmptyEntries, setShowEmptyEntries] = useState(false);

    const allEntries = useMemo(() => {
        if (entries) {
            return [
                ...entries.commits
                    .map((e) => ({
                        ...e,
                        type: "Commit",
                        icon: faCodeCommit
                    }))
                    .filter(
                        (e) =>
                            showEmptyEntries ||
                            e.added.length > 0 ||
                            e.deleted.length > 0 ||
                            e.moved.length > 0 ||
                            e.copied.length > 0
                    ),
                ...entries.syncs
                    .map((e) => ({
                        ...e,
                        type: "Sync",
                        icon: faArrowsRotate
                    }))
                    .filter(
                        (e) =>
                            showEmptyEntries ||
                            e.added.length > 0 ||
                            e.deleted.length > 0
                    ),
                ...entries.deploys
                    .map((e) => ({
                        ...e,
                        type: "Deploy",
                        icon: faRocket
                    }))
                    .filter((e) => showEmptyEntries || e.added.length > 0)
            ].sort((a, b) => b.time.getTime() - a.time.getTime());
        }
        return [];
    }, [entries, showEmptyEntries]);

    useEffect(() => {
        if (!entries) {
            return;
        }

        setNumOfPages(Math.ceil(allEntries.length / entriesPerPage));

        if (selectedPage > numOfPages) {
            setSelectedPage(numOfPages);
        }

        if (selectedPage < 1) {
            setSelectedPage(1);
        }
    }, [entries, numOfPages, selectedPage]);

    const [fullText, setFullText] = useState<string>();

    useEffect(() => {
        if (dateRange.since && dateRange.until) {
            updateEntries(
                new Date(dateRange.since),
                new Date(dateRange.until),
                fullText
            );
        }
    }, [dateRange, fullText, updateEntries]);

    const renderContent = () => {
        if (isLoading) {
            return (
                <div className="grow flex w-full justify-center content-center">
                    <Loading variant="bars" className="w-20" />
                </div>
            );
        }

        if (
            !entries ||
            (entries?.commits.length === 0 &&
                entries?.deploys.length === 0 &&
                entries?.syncs.length === 0)
        ) {
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
                            No events was found according to the selected
                            filters.
                        </Card.Body>
                    </Card>
                </div>
            );
        }

        return (
            <LogTable
                allEntries={allEntries!.slice(
                    (selectedPage - 1) * entriesPerPage,
                    selectedPage * entriesPerPage
                )}
                onRowClick={(e: LogEntry) => {
                    setModalEntry(e);
                    modalRef.current?.showModal();
                }}
            />
        );
    };
    const [modalEntry, setModalEntry] = useState<LogEntry>();

    const modalRef = useRef<HTMLDialogElement>(null);

    return (
        <div className="flex flex-col w-full h-full overflow-x-auto bg-base-200">
            <LogEntryModal
                ref={modalRef}
                logData={modalEntry}
                backdrop={true}
            />
            <Card className="text-sm text-gray-500 ransition bg-base-100 shadow-sm my-1 mx-10 mt-0 rounded-t-none">
                <Card.Body className="p-4">
                    <div className="flex justify-between items-center gap-3">
                        <span className="flex items-center w-44">
                            <FontAwesomeIcon className="w-5" icon={faClock} />
                            <h2 className="w-16 text-center align-middle">
                                Time
                            </h2>
                            <div className="grow" />
                        </span>
                        <Divider horizontal className="mx-1" />
                        <span className="flex items-center w-24">
                            <FontAwesomeIcon className="w-5" icon={faList} />
                            <h2 className="w-16 text-center align-middle">
                                Type
                            </h2>
                        </span>
                        <Divider horizontal className="mx-1" />
                        <FontAwesomeIcon className="w-5" icon={faCubes} />
                        <h2 className="w-16 text-center">Contents</h2>
                        <div className="grow" />
                        <Divider horizontal className="mx-1" />
                        <div className="input m-[-12px] rounded-xl flex items-center justify-start gap-3">
                            <FontAwesomeIcon className="w-5" icon={faSearch} />
                            <input
                                className="align-middle"
                                placeholder="Search..."
                                onChange={(e) => {
                                    setFullText(e.target.value);
                                }}
                            />
                        </div>
                    </div>
                </Card.Body>
            </Card>
            {renderContent()}

            <div className="grow" />
            <div className="flex justify-between items-center gap-3 sticky height-10 bottom-0 mx-10">
                <Card className="bg-base-100 shadow-sm overflow-clip rounded-b-none">
                    <Card.Body className="p-0 flex flex-row gap-3 items-center align-middle">
                        <DateRangeInput
                            className="mx-1"
                            since={subDays(new Date(), 7)}
                            until={new Date()}
                            onDateChange={(since, until) => {
                                setDateRange({
                                    since,
                                    until
                                });
                                updateEntries(
                                    new Date(dateRange.since),
                                    new Date(dateRange.until),
                                    fullText
                                );
                            }}
                        />
                        <div
                            className="flex items-center gap-1 px-3 select-none"
                            onClick={() =>
                                setShowEmptyEntries(!showEmptyEntries)
                            }
                        >
                            <Checkbox
                                size="sm"
                                onChange={(e) =>
                                    setShowEmptyEntries(e.target.checked)
                                }
                                checked={showEmptyEntries}
                            />
                            Show empty entries
                        </div>
                    </Card.Body>
                </Card>
                <Card className="bg-base-100 shadow-sm overflow-clip rounded-b-none">
                    <Card.Body className="p-0">
                        <Pagination className="rounded-b-none">
                            {[...Array(numOfPages).keys()].map((_, index) => (
                                <Button
                                    color="ghost"
                                    active={index == selectedPage - 1}
                                    onClick={() => setSelectedPage(index + 1)}
                                    key={index}
                                    className="join-item"
                                >
                                    {index + 1}
                                </Button>
                            ))}
                        </Pagination>
                    </Card.Body>
                </Card>
            </div>
        </div>
    );
}
