/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { DeployLogEntry } from "../../../definitions/log";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faPlusCircle, faRobot } from "@fortawesome/free-solid-svg-icons";

export function DeployRow({ entry }: { entry: DeployLogEntry }) {
    return (
        <>
            {entry.added.length > 0 && (
                <span>
                    <FontAwesomeIcon className="mr-2" icon={faPlusCircle} />
                    {entry.added.length} added
                </span>
            )}
            <div className="grow" />
            <h2>{entry.runnerUrl}</h2>
            <FontAwesomeIcon className="w-5" icon={faRobot} />
        </>
    );
}
