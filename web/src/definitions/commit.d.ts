/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

type AddAction = Map<string, Partial<PackageUpload>>;
type DeleteAction = Set<string>;
type TransferAction = Map<string, Section>;

type ActionType = "add" | "delete" | "copy" | "move";

type Commit = {
    toAdd: AddAction;
    toDelete: DeleteAction;
    toCopy: TransferAction;
    toMove: TransferAction;
};

type Commits = Map<string, Commit>;
