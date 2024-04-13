#include "Writer.h"

namespace Archive {

Writer::Result<void> Writer::open_filename(const std::filesystem::path& path) {
    const int status =
        archive_write_open_filename(m_archive.get(), path.c_str());

    if (status != ARCHIVE_OK) {
        return std::unexpected(LibArchiveError(m_archive.get()));
    }

    return {};
}

Writer::Result<void> Writer::open_memory(std::vector<std::byte>& byte_array,
                                         size_t& used_size) {
    const int status = archive_write_open_memory(
        m_archive.get(), byte_array.data(), byte_array.size(), &used_size);

    if (status != ARCHIVE_OK) {
        return std::unexpected(LibArchiveError(m_archive.get()));
    }

    return {};
}

Writer::Result<Writer::Entry> Writer::start_write(Header& header) {
    const int status = archive_write_header(m_archive.get(), header.entry());

    if (status != ARCHIVE_OK) {
        return std::unexpected(LibArchiveError(m_archive.get()));
    }

    Entry entry;
    entry.m_writer = m_archive.get();

    return entry;
}

Writer::Entry::Result<void>
    Writer::Entry::write(const std::vector<uint8_t>& data) {
    const auto status = archive_write_data(m_writer, data.data(), data.size());

    if (static_cast<int64_t>(status) < 0) {
        return std::unexpected(LibArchiveError(m_writer));
    }

    return {};
}

Writer::Entry::Result<void>
    Writer::Entry::operator>>(const std::vector<uint8_t>& data) {
    return write(data);
}

} // namespace Archive
