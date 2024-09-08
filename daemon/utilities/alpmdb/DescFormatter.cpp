/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#include "DescFormatter.h"

#include "utilities/base64.h"
#include "utilities/hash_from_file.h"

#include <sstream>

namespace bxt::Utilities::AlpmDb {

// This function formats package information into a desc contents string
// It follows the format used by repo-add.sh in pacman for compatibility:
// https://gitlab.archlinux.org/pacman/pacman/-/blob/6ba5c20e7629ae9bdd7ceaf5a45484c434363ec5/scripts/repo-add.sh.in#L296-326
std::string DescFormatter::format() const {
    std::ostringstream oss;
    oss << format_entry<"FILENAME">(m_filepath.filename().string());
    oss << format_pkginfo_entry<"NAME", "pkgname">();
    oss << format_pkginfo_entry<"BASE", "pkgbase">();
    oss << format_pkginfo_entry<"VERSION", "pkgver">();
    oss << format_pkginfo_entry<"DESC", "pkgdesc">();
    oss << format_pkginfo_entry<"GROUPS", "groups">();
    oss << format_entry<"CSIZE">(std::to_string(std::filesystem::file_size(m_filepath)));
    oss << format_pkginfo_entry<"ISIZE", "size">();

    // add checksums
    oss << format_entry<"MD5SUM">(bxt::hash_from_file<MD5, MD5_DIGEST_LENGTH>(m_filepath.string()));

    oss << format_entry<"SHA256SUM">(
        bxt::hash_from_file<SHA256, SHA256_DIGEST_LENGTH>(m_filepath.string()));

    // add PGP sig
    if (!m_signature.empty()) {
        oss << format_entry<"PGPSIG">(bxt::Utilities::b64_encode(m_signature));
    }

    oss << format_pkginfo_entry<"URL", "url">();
    oss << format_pkginfo_entry<"LICENSE", "license">();
    oss << format_pkginfo_entry<"ARCH", "arch">();
    oss << format_pkginfo_entry<"BUILDDATE", "builddate">();
    oss << format_pkginfo_entry<"PACKAGER", "packager">();
    oss << format_pkginfo_entry<"REPLACES", "replaces">();
    oss << format_pkginfo_entry<"CONFLICTS", "conflict">();
    oss << format_pkginfo_entry<"PROVIDES", "provides">();
    oss << format_pkginfo_entry<"DEPENDS", "depend">();
    oss << format_pkginfo_entry<"OPTDEPENDS", "optdepend">();
    oss << format_pkginfo_entry<"MAKEDEPENDS", "makedepend">();
    oss << format_pkginfo_entry<"CHECKDEPENDS", "checkdepend">();
    return oss.str();
}
} // namespace bxt::Utilities::AlpmDb
