/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <archive_entry.h>
#include <chrono>
#include <memory>

namespace Archive {
class Header {
public:
    Header() = default;
    Header(Header const& lh)
        : m_entry({archive_entry_clone(lh.m_entry.get()), lh.m_entry.get_deleter()}) {
    }
    Header(Header&& lh)
        : m_entry(std::move(lh.m_entry.release()), std::move(lh.m_entry.get_deleter())) {
    }

    explicit Header(archive_entry* entry)
        : m_entry(std::move(entry), [](archive_entry*) {}) {
    }

    static Header default_file() {
        Header header;

        archive_entry_set_filetype(header, AE_IFREG);
        archive_entry_set_mtime(
            header, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), 0);
        archive_entry_set_perm(header, 0644);

        archive_entry_set_gid(header, getgid());
        archive_entry_set_uid(header, getuid());

        return header;
    }

    operator archive_entry*() {
        return m_entry.get();
    }
    operator archive_entry const*() const {
        return m_entry.get();
    }

    archive_entry* entry() {
        return m_entry.get();
    }
    archive_entry const* entry() const {
        return m_entry.get();
    }

    void operator=(Header const& rh) {
        m_entry = {archive_entry_clone(rh.m_entry.get()), rh.m_entry.get_deleter()};
    }

    void operator=(Header&& rh) {
        m_entry = {std::move(rh.m_entry.release()), std::move(rh.m_entry.get_deleter())};
    }

private:
    std::unique_ptr<archive_entry, decltype(&archive_entry_free)> m_entry {archive_entry_new(),
                                                                           &archive_entry_free};
};

} // namespace Archive
