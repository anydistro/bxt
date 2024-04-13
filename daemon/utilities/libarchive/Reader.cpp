/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "Reader.h"

#include "utilities/libarchive/Error.h"

#include <archive.h>
#include <variant>

namespace Archive {
Reader::Result<void> Reader::open_filename(const std::filesystem::path &path) {
    int status =
        archive_read_open_filename(m_archive.get(), path.c_str(), 1024);

    if (status != ARCHIVE_OK) {
        return std::unexpected(LibArchiveError(m_archive.get()));
    }

    return {};
}

Reader::Result<void>
    Reader::open_memory(const std::vector<uint8_t> &byte_array) {
    int status = archive_read_open_memory(m_archive.get(), byte_array.data(),
                                          byte_array.size());

    if (status != ARCHIVE_OK) {
        return std::unexpected(LibArchiveError(m_archive.get()));
    }

    return {};
}

Reader::Result<void> Reader::open_memory(uint8_t *data, size_t length) {
    int status = archive_read_open_memory(m_archive.get(), data, length);

    if (status != ARCHIVE_OK) {
        return std::unexpected(LibArchiveError(m_archive.get()));
    }

    return {};
}

Reader::Entry::Result<std::vector<uint8_t>> Reader::Entry::read_all() {
    constexpr int block_size = 1024;
    std::array<uint8_t, block_size> buffer;

    std::size_t actual_size = 0;
    std::vector<uint8_t> result;

    do {
        const auto read_result = read_buffer(buffer, actual_size);

        if (!read_result.has_value()) {
            return std::unexpected(read_result.error());
        }

        std::copy(buffer.begin(), buffer.begin() + actual_size,
                  std::back_inserter(result));
    } while (actual_size == block_size);

    return result;
}

Reader::Entry::Result<std::vector<uint8_t>>
    Reader::Entry::read(std::size_t amount) {
    if (!m_reader) { return std::unexpected(InvalidEntryError()); }

    std::vector<uint8_t> result;
    result.resize(amount);

    const auto size = archive_read_data(m_reader, result.data(), amount);

    if (static_cast<int64_t>(size) < 0) {
        return std::unexpected(LibArchiveError(m_reader));
    }

    result.resize(size);

    return result;
}

} // namespace Archive
