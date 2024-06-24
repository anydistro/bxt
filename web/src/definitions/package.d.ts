/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
type PackagePoolEntry = {
    version: string;
    filepath: string;
    signaturePath?: string;
};

type Package = {
    section: Section;
    name: string;
    isAnyArchitecture?: boolean;
    preferredLocation?: string;
    poolEntries?: {
        [key: string]: PackagePoolEntry;
    };
};

type PackageUpload = {
    section: Section;
    name: string;
    version?: string;
    file: File;
    signatureFile: File;
};
