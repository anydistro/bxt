
/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
// Local
#include "validation.h"

// bxt
#include <persistence/box/record/PackageRecord.h>
#include <utilities/lmdb/CerealSerializer.h>
#include <utilities/MemoryLiterals.h>
#include <utilities/to_string.h>

// External
#include <CLI/CLI.hpp>
#include <coro/io_scheduler.hpp>
#include <coro/sync_wait.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/std.h>
#include <lmdbxx/lmdb++.h>

// STL
#include <string>

namespace bxt::cli {

namespace {
    using namespace bxt::MemoryLiterals;
    constexpr size_t LmdbMaxDbs = 128;
    constexpr size_t LmdbMapSize = 50_GiB;
} // namespace

using Serializer = bxt::Utilities::LMDB::CerealSerializer<bxt::Persistence::Box::PackageRecord>;
namespace handlers {
    int list(lmdb::txn& transaction, lmdb::dbi& db, std::string const& prefix) {
        auto cursor = lmdb::cursor::open(transaction, db);
        if (!prefix.empty()) {
            std::string_view key;
            if (!cursor.get(key, MDB_SET_RANGE) || !key.starts_with(prefix)) {
                fmt::print(stderr, "No packages found with prefix {}\n", prefix);
                return 1;
            }
            while (cursor.get(key, MDB_NEXT) && key.starts_with(prefix)) {
                fmt::print("{}\n", key);
            }
        } else {
            std::string_view key;
            while (cursor.get(key, MDB_NEXT)) {
                fmt::print("{}\n", key);
            }
        }
        return 0;
    }

    int get(lmdb::txn& transaction, lmdb::dbi& db, std::string const& key) {
        std::string_view data;
        auto result = db.get(transaction, key, data);
        if (!result) {
            fmt::print(stderr, "Failed to retrieve value or value not found.\n");
            return 1;
        }

        auto const package = Serializer::deserialize(std::string(data));
        if (!package.has_value()) {
            fmt::print(stderr, "Failed to deserialize package.\n");
            return 1;
        }

        fmt::print("{}\nIs any arch: {}\nDescriptions:\n", package->id.to_string(),
                   package->is_any_architecture);
        for (auto const& [key, value] : package->descriptions) {
            fmt::print("==={}===\nFilepath: {}\nSignature path: "
                       "{}\nDescfile:\n\n{}\n",
                       bxt::to_string(key), value.filepath.string(), value.signature_path->string(),
                       value.descfile.desc);
        }
        return 0;
    }

    int delete_(lmdb::txn& transaction, lmdb::dbi& db, std::string const& key) {
        auto result = db.del(transaction, key);
        if (result) {
            fmt::print("Value deleted successfully.\n");
            return 0;
        } else {
            fmt::print(stderr, "Failed to delete value or value not found.\n");
            return 1;
        }
    }

    int validate(lmdb::txn& transaction, lmdb::dbi& db) {
        Validator validator(transaction, db, false, false);
        auto error_count = validator.validate_and_rebuild();
        if (error_count == 0) {
            fmt::print("No errors found.\n");
        } else {
            fmt::print("{} errors found.\n", error_count);
        }
        return error_count > 0 ? 1 : 0;
    }

    int rebuild(lmdb::txn& transaction, lmdb::dbi& db, bool rebuild_keys) {
        Validator validator(transaction, db, true, rebuild_keys);

        if (validator.validate_and_rebuild() == 0) {
            fmt::print("Successfully rebuilt{} packages.\n",
                       rebuild_keys ? " all package keys" : "");
            transaction.commit();
            return 0;
        } else {
            fmt::print(stderr, "Failed to rebuild packages.\n");
            return 1;
        }
    }
} // namespace handlers

class DatabaseCli {
public:
    int run(int argc, char** argv) {
        CLI::App app {"BXT Database CLI Tool"};
        app.require_subcommand(1);

        std::string prefix;
        auto list = app.add_subcommand("list", "List packages");
        list->add_option("prefix", prefix, "Package name prefix to filter by");

        std::string get_key;
        auto get = app.add_subcommand("get", "Get package details");
        get->add_option("key", get_key, "Package key to retrieve")->required();

        std::string del_key;
        auto del = app.add_subcommand("del", "Delete package");
        del->add_option("key", del_key, "Package key to delete")->required();

        auto validate = app.add_subcommand("validate", "Validate database records");

        bool rebuild_keys = false;
        auto rebuild = app.add_subcommand("rebuild", "Rebuild database records");
        rebuild->add_flag("--keys", rebuild_keys, "Rebuild package keys");

        CLI11_PARSE(app, argc, argv);

        auto lmdbenv = lmdb::env::create();
        lmdbenv.set_mapsize(LmdbMapSize);
        lmdbenv.set_max_dbs(LmdbMaxDbs);
        std::error_code ec;

        if (!std::filesystem::exists("./bxtd.lmdb", ec) || ec) {
            fmt::print(stderr, "bxtd.lmdb not found\n");
            return 1;
        }

        lmdbenv.open("./bxtd.lmdb");
        auto transaction = lmdb::txn::begin(lmdbenv);
        auto db = lmdb::dbi::open(transaction, "bxt::Box");

        if (list->parsed()) {
            return handlers::list(transaction, db, prefix);
        } else if (get->parsed()) {
            return handlers::get(transaction, db, get_key);
        } else if (del->parsed()) {
            return handlers::delete_(transaction, db, del_key);
        } else if (validate->parsed()) {
            return handlers::validate(transaction, db);
        } else if (rebuild->parsed()) {
            return handlers::rebuild(transaction, db, rebuild_keys);
        }

        return 0;
    }
};

} // namespace bxt::cli

int main(int argc, char** argv) {
    return bxt::cli::DatabaseCli {}.run(argc, argv);
}
