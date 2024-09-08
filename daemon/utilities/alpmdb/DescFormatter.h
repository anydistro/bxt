/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "utilities/alpmdb/PkgInfo.h"
#include "utilities/FixedString.h"

#include <boost/algorithm/string/join.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <string>

namespace bxt::Utilities::AlpmDb {
class DescFormatter {
public:
    DescFormatter(PkgInfo m_pkg_info, std::filesystem::path m_filepath, std::string m_signature)
        : m_pkg_info(std::move(m_pkg_info))
        , m_filepath(std::move(m_filepath))
        , m_signature(std::move(m_signature)) {
    }

    static constexpr char format_string[] = "%{}%\n{}\n\n";

    template<FixedString desc_field, FixedString pkginfo_field>
    std::string format_pkginfo_entry() const {
        auto values = m_pkg_info.values(pkginfo_field.buf);
        if (values.empty()) {
            return "";
        }

        return fmt::format(format_string, desc_field.buf, boost::join(values, "\n"));
    }
    template<FixedString desc_field> std::string format_entry(std::string const& value) const {
        if (value.empty())
            return "";
        return fmt::format(format_string, desc_field.buf, value);
    }

    std::string format() const;

private:
    PkgInfo m_pkg_info;
    std::filesystem::path m_filepath;
    std::string m_signature;
};

} // namespace bxt::Utilities::AlpmDb
