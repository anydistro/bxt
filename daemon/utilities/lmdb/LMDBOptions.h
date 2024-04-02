/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "utilities/configuration/Configuration.h"

#include <filesystem>
namespace bxt::Utilities::LMDB {

struct LMDBOptions {
    virtual ~LMDBOptions() = default;
    std::filesystem::path lmdb_path = "bxtd.lmdb";

    void serialize(Configuration& config) {
        config.set("lmdb-path", lmdb_path.string());
    }
    void deserialize(const Configuration& config) {
        lmdb_path = config.get<std::string>("lmdb-path").value_or(lmdb_path);
    }
};

} // namespace bxt::Utilities::LMDB
