/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include <archive_entry.h>
#include <memory>

namespace Archive {
class Header
{
public:
    Header() = default;
    Header(const Header& lh)
        : m_entry(
            {archive_entry_clone(lh.m_entry.get()), &archive_entry_free}) {}
    Header(Header&& lh)
        : m_entry(std::move(lh.m_entry.release()), &archive_entry_free) {}

    operator archive_entry*() { return m_entry.get(); }
    operator const archive_entry*() const { return m_entry.get(); }

    archive_entry* entry() { return m_entry.get(); }
    const archive_entry* entry() const { return m_entry.get(); }

    void operator=(const Header& lh) {
        m_entry = {archive_entry_clone(lh.m_entry.get()), &archive_entry_free};
    }

private:
    std::unique_ptr<archive_entry, decltype(&archive_entry_free)> m_entry {
        archive_entry_new(), &archive_entry_free};
};

} // namespace Archive
