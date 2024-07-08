/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { SyncLogEntry } from "../../../definitions/log";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    faPlusCircle,
    faTrash,
    faUser
} from "@fortawesome/free-solid-svg-icons";

export function SyncRow({ entry }: { entry: SyncLogEntry }) {
    return (
        <>
            {entry.added.length > 0 && (
                <span>
                    <FontAwesomeIcon className="mr-2" icon={faPlusCircle} />
                    {entry.added.length} added
                </span>
            )}
            {entry.deleted.length > 0 && (
                <span>
                    <FontAwesomeIcon className="mr-2" icon={faTrash} />
                    {entry.deleted.length} deleted
                </span>
            )}
            <div className="grow" />
            <h2>{entry.syncTriggerUsername}</h2>
            <FontAwesomeIcon className="w-5" icon={faUser} />
        </>
    );
}
