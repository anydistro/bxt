/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <cereal/specialize.hpp>
#include <filesystem>
#include <string>
namespace std::filesystem {

template<class Archive>
void CEREAL_LOAD_MINIMAL_FUNCTION_NAME(Archive const&, path& out, string const& in) {
    out = in;
}

template<class Archive> string CEREAL_SAVE_MINIMAL_FUNCTION_NAME(Archive const& ar, path const& p) {
    return p.string();
}

} // namespace std::filesystem

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(std::filesystem::path,
                                   cereal::specialization::non_member_load_save_minimal);
