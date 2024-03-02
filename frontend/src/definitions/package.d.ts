/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
interface IPackagePoolEntry {
    version: string;
    filepath: string;
    signaturePath?: string;
}

interface IPackage {
    section: ISection;
    name: string;
    isAnyArchitecture?: boolean;
    preferredCandidate?: IPackagePoolEntry;
    poolEntries?: {
        [key: string]: IPackagePoolEntry;
    };
}

interface IPackageUpload {
    section: ISection;
    name: string;
    version?: string;
    file: File;
    signatureFile?: File;
}
