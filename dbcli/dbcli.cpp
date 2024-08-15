/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "persistence/box/record/PackageRecord.h"
#include "utilities/lmdb/CerealSerializer.h"
#include "utilities/to_string.h"

#include <coro/io_scheduler.hpp>
#include <coro/sync_wait.hpp>
#include <cstdio>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fstream>
#include <lmdbxx/lmdb++.h>
#include <string>

using Serializer = bxt::Utilities::LMDB::CerealSerializer<
    bxt::Persistence::Box::PackageRecord>;

int validate_and_rebuild(lmdb::txn& transaction,
                         lmdb::dbi& db,
                         bool rebuild_descfile = false) {
    auto cursor = lmdb::cursor::open(transaction, db);
    std::string_view key, value;

    int error_count = 0;

    while (cursor.get(key, value, MDB_NEXT)) {
        auto record = Serializer::deserialize(std::string(value));
        if (!record) {
            fmt::print(stderr, fg(fmt::terminal_color::red),
                       "{}: Failed to deserialize record: {}\n",
                       record->id.to_string(), record.error().what());
            if (rebuild_descfile) { return 1; }
            error_count++;
            continue;
        }

        bool needs_update = false;

        for (auto& [location, description] : record->descriptions) {
            if (!std::filesystem::exists(description.filepath)) {
                fmt::print(stderr, fg(fmt::terminal_color::red),
                           "{} ({}): File not found: {}\n",
                           record->id.to_string(), bxt::to_string(location),
                           description.signature_path->string());
                if (rebuild_descfile) { return 1; }
                error_count++;
                continue;
            }

            if (description.signature_path
                && !std::filesystem::exists(*description.signature_path)) {
                fmt::print(stderr, fg(fmt::terminal_color::red),
                           "{} ({}): Signature file not found: {}\n",
                           record->id.to_string(), bxt::to_string(location),
                           description.signature_path->string());
                if (rebuild_descfile) { return 1; }
                error_count++;
                continue;
            }

            std::string signature;

            if (description.signature_path) {
                try {
                    std::ifstream file(*description.signature_path);
                    if (file.is_open()) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        signature = buffer.str();
                        file.close();
                    } else {
                        std::cerr << "Unable to open signature file: "
                                  << *description.signature_path << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error reading signature file: " << e.what()
                              << std::endl;
                }
            }

            auto desc_result = bxt::Utilities::AlpmDb::Desc::parse_package(
                description.filepath, signature);

            if (!desc_result) {
                fmt::print(stderr, fg(fmt::terminal_color::red),
                           "{} ({}): Failed to parse desc-file for: {}\n",
                           record->id.to_string(), bxt::to_string(location),
                           description.filepath.string());
                if (rebuild_descfile) { return 1; }
                error_count++;
                continue;
            }
            fmt::print(fg(fmt::terminal_color::green), "{} ({}): Valid\n",
                       record->id.to_string(), bxt::to_string(location));

            if (rebuild_descfile) {
                fmt::print("{} ({}): Rebuilding desc-file for: {}\n",
                           record->id.to_string(), bxt::to_string(location),
                           record->id.to_string());
                description.descfile = std::move(*desc_result);
                needs_update = true;
            }
        }

        if (needs_update) {
            auto serialized = Serializer::serialize(*record);
            if (!serialized) {
                fmt::print(stderr, fg(fmt::terminal_color::red),
                           "{}: Failed to serialize record: {}\n",
                           record->id.to_string(), serialized.error().what());
                return 1;
            }

            try {
                db.put(transaction, key, *serialized);
            } catch (const std::exception& e) {
                fmt::print(stderr, fg(fmt::terminal_color::red),
                           "{}: Failed to update record: {}\n",
                           record->id.to_string(), e.what());
                return 1;
            }
        }
    }
    return error_count;
}

int main(int argc, char** argv) {
    auto lmdbenv = lmdb::env::create();

    lmdbenv.set_mapsize(50UL * 1024UL * 1024UL * 1024UL);
    lmdbenv.set_max_dbs(128);
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
    } else if (command == "validate") {
        if (argc != 2) {
            fmt::print(stderr, "Usage: {} validate\n", argv[0]);
            return 1;
        }

        auto error_count = validate_and_rebuild(transaction, db, false);

        if (error_count == 0) {
            fmt::print("No errors found.\n");
        } else {
            fmt::print("{} errors found.\n", error_count);
        }

    } else if (command == "rebuild") {
        if (argc != 2) {
            fmt::print(stderr, "Usage: {} rebuild\n", argv[0]);
            return 1;
        }
        if (validate_and_rebuild(transaction, db, true) == 0) {
            fmt::print("Successfully rebuilt all packages.\n");
            transaction.commit();
        }

    } else {
        fmt::print(stderr, "Unknown command: {}\n", command);
        return 1;
    }

    return 0;
}
