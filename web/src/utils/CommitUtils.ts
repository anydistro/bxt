/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

export function mergeCommits(
    commit: Commit,
    partialCommit: Partial<Commit>
): Commit {
    const mergedCommit: Commit = {
        toAdd: new Map([...commit.toAdd, ...(partialCommit.toAdd || [])]),
        toDelete: new Set([
            ...commit.toDelete,
            ...(partialCommit.toDelete || [])
        ]),
        toCopy: new Map([...commit.toCopy, ...(partialCommit.toCopy || [])]),
        toMove: new Map([...commit.toMove, ...(partialCommit.toMove || [])])
    };

    const allKeys = [
        ...mergedCommit.toAdd.keys(),
        ...mergedCommit.toDelete,
        ...mergedCommit.toCopy.keys(),
        ...mergedCommit.toMove.keys()
    ];
    const uniqueKeys = new Set(allKeys);
    if (allKeys.length !== uniqueKeys.size) {
        throw new Error("Commit contains duplicate keys");
    }

    return mergedCommit;
}

export function createCommit(commit: Partial<Commit> = {}): Commit {
    return {
        toAdd: new Map(),
        toDelete: new Set(),
        toCopy: new Map(),
        toMove: new Map(),
        ...commit
    };
}
