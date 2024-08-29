/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace bxt {

template<auto HashFunction, size_t DigestLength>
const std::string hash_from_file(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path) || std::filesystem::is_empty(path)) {
        return "";
    }

    std::array<unsigned char, DigestLength> result;
    boost::iostreams::mapped_file_source src(path);
    HashFunction(reinterpret_cast<const unsigned char *>(src.data()),
                 src.size(), result.data());

    std::ostringstream sout;
    sout << std::hex << std::setfill('0');
    for (auto c : result)
        sout << std::setw(2) << (int)c;

    return sout.str();
}
} // namespace bxt
