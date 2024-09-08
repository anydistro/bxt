/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/Error.h"

#include <archive.h>
#include <exception>
#include <string>

namespace Archive {

struct ArchiveError : public bxt::Error {
    ArchiveError() {
        message = "Unknown Archive error";
    }
};

struct InvalidEntryError : public ArchiveError {
    InvalidEntryError() {
        message = "The entry has no linked archive!";
    }
};

struct LibArchiveError : public ArchiveError {
    explicit LibArchiveError(archive* archive) {
        message = archive_error_string(archive);
    }
};
} // namespace Archive
