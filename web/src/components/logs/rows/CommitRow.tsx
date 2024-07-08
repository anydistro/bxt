/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { CommitLogEntry } from "../../../definitions/log";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    faArrowsUpDownLeftRight,
    faCopy,
    faPlusCircle,
    faTrash,
    faUser
} from "@fortawesome/free-solid-svg-icons";

export function CommitRow({ entry }: { entry: CommitLogEntry }) {
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
                    <FontAwesomeIcon className="mr-2" icon={faPlusCircle} />
                    {entry.deleted.length} deleted
                </span>
            )}

            {entry.moved.length > 0 && (
                <span>
                    <FontAwesomeIcon
                        className="mr-2"
                        icon={faArrowsUpDownLeftRight}
                    />
                    {entry.moved.length} moved
                </span>
            )}
            {entry.copied.length > 0 && (
                <span>
                    <FontAwesomeIcon className="mr-2" icon={faCopy} />
                    {entry.copied.length} copied
                </span>
            )}

            <div className="grow" />
            <h2>{entry.commiterUsername}</h2>
            <FontAwesomeIcon className="w-5" icon={faUser} />
        </>
    );
}
