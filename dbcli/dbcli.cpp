/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "persistence/box/record/PackageRecord.h"
#include "utilities/lmdb/CerealSerializer.h"

#include <coro/io_scheduler.hpp>
#include <coro/sync_wait.hpp>
#include <cstdio>
#include <fmt/core.h>
#include <lmdbxx/lmdb++.h>
#include <string>

using Serializer = bxt::Utilities::LMDB::CerealSerializer<
    bxt::Persistence::Box::PackageRecord>;

int main(int argc, char** argv) {
    auto lmdbenv = lmdb::env::create();

    lmdbenv.set_mapsize(1UL * 1024UL * 1024UL * 1024UL);
    lmdbenv.set_max_dbs(10);
    std::error_code ec;

    if (!std::filesystem::exists("./bxtd.lmdb", ec) || ec) {
        fmt::print(stderr, "bxtd.lmdb not found\n");
        return 1;
    }

    lmdbenv.open("./bxtd.lmdb");

    auto transaction = lmdb::txn::begin(lmdbenv);
    auto db = lmdb::dbi::open(transaction, "bxt::Box");

    if (argc < 2) {
        fmt::print("Usage: {} <command> [options]\n", argv[0]);
        return 1;
    }

    std::string command = argv[1];
    if (command == "list") {
        auto cursor = lmdb::cursor::open(transaction, db);
        if (argc >= 3) {
            std::string_view key = argv[2];
            if (!cursor.get(key, MDB_SET_RANGE) || !key.starts_with(argv[2])) {
                fmt::print(stderr, "No packages found with prefix {}\n", key);
                return 1;
            }
            do {
                fmt::print("{}\n", key);
            } while (cursor.get(key, MDB_NEXT) && key.starts_with(argv[2]));
        } else {
            std::string_view key;
            while (cursor.get(key, MDB_NEXT)) {
                fmt::print("{}\n", key);
            }
        }
    } else if (command == "get") {
        if (argc != 3) {
            fmt::print(stderr, "Usage: {} get <key>\n", argv[0]);
            return 1;
        }

        std::string key = argv[2];
        std::string_view data;
        auto result = db.get(transaction, key, data);
        if (!result) {
            fmt::print(stderr,
                       "Failed to retrieve value or value not found.\n");
            return 1;
        }

        const auto package = Serializer::deserialize(std::string(data));
        if (!package.has_value()) {
            fmt::print(stderr, "Failed to deserialize package.\n");
            return 1;
        }

        fmt::print("{}\nIs any arch: {}\nDescriptions:\n",
                   package->id.to_string(), package->is_any_architecture);
        for (const auto& [key, value] : package->descriptions) {
            fmt::print("==={}===\nFilepath: {}\nSignature path: "
                       "{}\nDescfile:\n\n{}\n",
                       bxt::to_string(key), value.filepath.string(),
                       value.signature_path->string(), value.descfile.desc);
        }
    } else if (command == "del") {
        if (argc != 3) {
            fmt::print(stderr, "Usage: {} del <key>\n", argv[0]);
            return 1;
        }

        std::string_view key = argv[2];
        auto result = db.del(transaction, key);
        if (result) {
            fmt::print("Value deleted successfully.\n", argv[0]);
        } else {
            fmt::print(stderr, "Failed to delete value or value not found.\n");
        }
    } else {
        fmt::print(stderr, "Unknown command: {}\n", command);
        return 1;
    }

    return 0;
}
