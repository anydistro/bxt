/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "Error.h"
#include "Header.h"
#include "utilities/errors/Macro.h"

#include <archive.h>
#include <expected>
#include <filesystem>
#include <memory>
#include <ostream>
#include <variant>
#include <vector>

struct archive;
struct archive_entry;

namespace Archive {

class Writer {
public:
    class Entry {
        friend class Writer;

    public:
        Entry() = default;

        template<typename T> using Result = std::expected<T, ArchiveError>;

        // Use the Result template for function return type
        Result<void> write(const std::vector<uint8_t>& data);
        Result<void> operator>>(const std::vector<uint8_t>& data);

        Result<void> finish() {
            if (!m_writer) { return std::unexpected(InvalidEntryError {}); }

            if (archive_write_finish_entry(m_writer) != ARCHIVE_OK) {
                return std::unexpected(LibArchiveError(m_writer));
            }
            return {};
        }

        const Header& header() { return m_header; }

    private:
        archive* m_writer = nullptr;
        Header m_header;
    };

    Writer() = default;

    BXT_DECLARE_RESULT(LibArchiveError)

    Result<void> open_filename(const std::filesystem::path& path);
    Result<void> open_memory(std::vector<std::byte>& byte_array,
                             size_t& used_size);

    operator archive*() { return m_archive.get(); }

    Result<Entry> start_write(Header& header);

private:
    static Result<void> deleter(archive* a) {
        const int status = archive_write_free(a);
        if (status != ARCHIVE_OK) {
            return std::unexpected(LibArchiveError(a));
        }
        return {};
    }
    std::unique_ptr<archive, decltype(&deleter)> m_archive {archive_write_new(),
                                                            &deleter};
};

} // namespace Archive
