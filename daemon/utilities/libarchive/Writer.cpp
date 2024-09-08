#include "Writer.h"

namespace Archive {

Writer::Result<void> Writer::open_filename(std::filesystem::path const& path) {
    int const status = archive_write_open_filename(m_archive.get(), path.c_str());

    if (status != ARCHIVE_OK) {
        return std::unexpected(LibArchiveError(m_archive.get()));
    }

    return {};
}

Writer::Result<void> Writer::open_memory(std::vector<std::byte>& byte_array, size_t& used_size) {
    int const status = archive_write_open_memory(m_archive.get(), byte_array.data(),
                                                 byte_array.size(), &used_size);

    if (status != ARCHIVE_OK) {
        return std::unexpected(LibArchiveError(m_archive.get()));
    }

    return {};
}

Writer::Result<Writer::Entry> Writer::start_write(Header& header) {
    int const status = archive_write_header(m_archive.get(), header.entry());

    if (status != ARCHIVE_OK) {
        return std::unexpected(LibArchiveError(m_archive.get()));
    }

    Entry entry;
    entry.m_writer = m_archive.get();

    return entry;
}

Writer::Entry::Result<void> Writer::Entry::write(std::vector<uint8_t> const& data) {
    auto const status = archive_write_data(m_writer, data.data(), data.size());

    if (static_cast<int64_t>(status) < 0) {
        return std::unexpected(LibArchiveError(m_writer));
    }

    return {};
}

Writer::Entry::Result<void> Writer::Entry::operator>>(std::vector<uint8_t> const& data) {
    return write(data);
}

} // namespace Archive
