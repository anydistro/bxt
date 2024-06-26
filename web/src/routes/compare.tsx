/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useState, useEffect } from "react";
import { useCompareResults, useSections } from "../hooks/BxtHooks";
import { Button, Card, Loading } from "react-daisyui";
import CompareTable from "../components/CompareTable";
import CompareInputForm from "../components/CompareInputForm";
import { faBan, faCodeCompare } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";

export default function Compare(props: any) {
    const [sections, updateSections] = useSections();
    const [compareResults, getCompareResults, resetCompareResults] =
        useCompareResults();

    const [isLoading, setIsLoading] = useState<boolean>(false);

    useEffect(() => {
        setIsLoading(false);
    }, [compareResults]);

    const renderContent = () => {
        if (!compareResults) {
            return (
                <CompareInputForm
                    sections={sections}
                    onSubmit={getCompareResults}
                />
            );
        }

        if (isLoading) {
            return (
                <div className="grow flex w-full justify-center content-center">
                    <Loading variant="bars" className="w-20" />
                </div>
            );
        }

        if (compareResults.sections.length > 0) {
            return (
                <CompareTable
                    compareResults={compareResults}
                    resetCompareResults={resetCompareResults}
                />
            );
        }

        return (
            <div className="overflow-y-auto flex h-full w-full justify-center items-center">
                <Card className="m-auto bg-base-100 shadow-sm" compact={true}>
                    <Card.Body>
                        <Card.Title>
                            <FontAwesomeIcon icon={faBan} />
                            Not found
                        </Card.Title>
                        The required packages are not found in the database.
                        <Card.Actions className="justify-end">
                            <Button
                                color="accent"
                                onClick={() => resetCompareResults()}
                                size="sm"
                            >
                                <FontAwesomeIcon icon={faCodeCompare} />
                                New compare
                            </Button>
                        </Card.Actions>
                    </Card.Body>
                </Card>
            </div>
        );
    };

    return (
        <div className="w-full h-full bg-base-200 overflow-x-auto">
            {renderContent()}
        </div>
    );
}
