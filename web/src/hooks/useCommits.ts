/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useCallback, useState } from "react";
import { SectionUtils } from "../utils/SectionUtils";

export type CommitsState = {
    commits: Map<string, Commit>;
    addCommit: (section: Section, commit: Commit) => void;
    deleteCommit: (section: Section) => void;
    clearCommits: () => void;
};

export default function useCommits(): CommitsState {
    const [commits, setCommits] = useState(new Map<string, Commit>());

    const addCommit = useCallback(
        (section: Section, commit: Commit) => {
            setCommits((commits) => {
                commits.set(SectionUtils.toString(section), commit);
                return new Map(commits);
            });
        },
        [setCommits]
    );
    const deleteCommit = useCallback(
        (section: Section) => {
            setCommits((commits) => {
                commits.delete(SectionUtils.toString(section));
                return new Map(commits);
            });
        },
        [setCommits]
    );

    const clearCommits = useCallback(() => {
        setCommits(new Map());
    }, [setCommits]);

    return { commits, addCommit, deleteCommit, clearCommits };
}
