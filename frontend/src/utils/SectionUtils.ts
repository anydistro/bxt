/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

const filteredValues = (
    sections: ISection[],
    filterFunction: (section: ISection) => boolean,
    valueExtractor: (section: ISection) => string | undefined
): string[] => {
    const valueSet = new Set<string>();
    sections.forEach((section) => {
        if (filterFunction(section)) {
            const value = valueExtractor(section);
            if (value) valueSet.add(value);
        }
    });

    return Array.from(valueSet);
};

export const branches = (sections: ISection[]): string[] => {
    return filteredValues(
        sections,
        () => true,
        (section) => section.branch
    );
};

export const reposForBranch = (
    sections: ISection[],
    branchName: string | undefined
): string[] => {
    return filteredValues(
        sections,
        (section) => section.branch === branchName,
        (section) => section.repository
    );
};

export const architecturesForBranchAndRepo = (
    sections: ISection[],
    branchName: string | undefined,
    repoName: string | undefined
): string[] => {
    return filteredValues(
        sections,
        (section) =>
            section.branch === branchName && section.repository === repoName,
        (section) => section.architecture
    );
};
