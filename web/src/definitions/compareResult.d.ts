/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
interface ICompareEntry {
    name: string;
    [section: string]: {
        [location: string]: string;
    };
}

interface ICompareResult {
    sections: ISection[];
    compareTable: ICompareEntry[];
}
