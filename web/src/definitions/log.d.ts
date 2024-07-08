/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

export enum LogEntryType {
    Add = "Add",
    Remove = "Remove",
    Update = "Update"
}
export type PackageLogEntry = {
    type: LogEntryType;
    section: Section;
    name: string;
    location: string;
    version?: string;
};

export type CommitLogEntry = {
    time: Date;
    commiterUsername: string;
    added: PackageLogEntry[];
    deleted: PackageLogEntry[];
    moved: [PackageLogEntry, PackageLogEntry][];
    copied: [PackageLogEntry, PackageLogEntry][];
};

export type SyncLogEntry = {
    time: Date;
    syncTriggerUsername: string;
    added: PackageLogEntry[];
    deleted: PackageLogEntry[];
};

export type DeployLogEntry = {
    time: Date;
    runnerUrl: string;
    added: PackageLogEntry[];
};

export type Log = {
    commits: CommitLogEntry[];
    syncs: SyncLogEntry[];
    deploys: DeployLogEntry[];
};

export type LogEntry = (CommitLogEntry | SyncLogEntry | DeployLogEntry) & {
    type: string;
    icon: IconProp;
};
