
/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

// bxt
#include <persistence/box/record/PackageRecord.h>
#include <utilities/lmdb/CerealSerializer.h>
#include <utilities/to_string.h>

// External
#include <coro/task.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/std.h>
#include <lmdbxx/lmdb++.h>

// STL
#include <fstream>
#include <sstream>
#include <string>

class Validator {
public:
    Validator(lmdb::txn& transaction, lmdb::dbi& db, bool rebuild_descfile, bool rebuild_keys)
        : m_transaction(transaction)
        , m_db(db)
        , m_rebuild_descfile(rebuild_descfile)
        , m_rebuild_keys(rebuild_keys) {
    }

    int validate_and_rebuild() {
        auto cursor = lmdb::cursor::open(m_transaction, m_db);
        std::string_view key, value;

        while (cursor.get(key, value, MDB_NEXT)) {
            validate_record(cursor, key, value);
        }

        return m_error_count;
    }

    int error_count() const {
        return m_error_count;
    }

    lmdb::dbi& db() const {
        return m_db;
    }

    lmdb::txn& transaction() const {
        return m_transaction;
    }

protected:
    std::string read_file_content(std::filesystem::path const& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::ios_base::failure("Unable to open file: " + filepath.string());
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    template<typename... TArgs>
    void handle_error(fmt::format_string<TArgs...> fmt, TArgs&&... args) {
        ++m_error_count;
        fmt::print(stderr, fg(fmt::terminal_color::red), fmt, std::forward<TArgs>(args)...);
    }

    bool validate_description(std::string const& record_id,
                              std::string const& location,
                              bxt::Persistence::Box::PackageRecord::Description& description) {
        namespace fs = std::filesystem;

        if (!fs::exists(description.filepath)) {
            handle_error("{} ({}): File not found: {}\n", record_id, location,
                         description.filepath);
            return m_rebuild_descfile;
        }

        if (description.signature_path && !fs::exists(*description.signature_path)) {
            handle_error("{} ({}): Signature file not found: {}\n", record_id, location,
                         *description.signature_path);
            return m_rebuild_descfile;
        }

        std::string signature;
        if (description.signature_path) {
            try {
                signature = read_file_content(*description.signature_path);
            } catch (std::exception const& e) {
                handle_error("{} ({}): Error reading signature file: {}\n", record_id, location,
                             e.what());
                return false;
            }
        }

        auto desc_result =
            bxt::Utilities::AlpmDb::Desc::parse_package(description.filepath, signature);
        if (!desc_result) {
            handle_error("{} ({}): Failed to parse desc-file: {}\n", record_id, location,
                         description.filepath.string());
            return m_rebuild_descfile;
        }

        if (desc_result->desc != description.descfile.desc) {
            fmt::print(stderr, fg(fmt::terminal_color::yellow), "{} ({}): Desc-file mismatch\n",
                       record_id, location);

            fs::path report_dir = "dbcli-report";
            fs::path package_dir = report_dir / record_id / location;
            fs::create_directories(package_dir);

            std::ofstream db_file(package_dir / "db");
            db_file << description.descfile.desc;

            std::ofstream pkg_file(package_dir / "pkg");
            pkg_file << desc_result->desc;

            fmt::print(fg(fmt::terminal_color::yellow), "{} ({}): Desc files saved in {}\n",
                       record_id, location, package_dir.string());

            if (!m_rebuild_descfile) {
                return false;
            }

            description.descfile = std::move(*desc_result);
        }

        fmt::print(fg(fmt::terminal_color::green), "{} ({}): Valid\n", record_id, location);
        return true;
    }

    void validate_record(lmdb::cursor& cursor, std::string_view key, std::string_view value) {
        fmt::print("Checking record: {}\n", key);

        auto record = Serializer::deserialize(std::string(value));
        if (!record) {
            handle_error("{}: Failed to deserialize record: {}\n", key, record.error().what());
            return;
        }

        bool needs_update = false;

        for (auto& [location, description] : record->descriptions) {
            if (!validate_description(record->id.to_string(), bxt::to_string(location),
                                      description)) {
                continue;
            }
            needs_update = m_rebuild_descfile;
        }

        if (needs_update) {
            auto serialized = Serializer::serialize(*record);
            if (!serialized) {
                handle_error("{}: Failed to serialize record: {}\n", record->id.to_string(),
                             serialized.error().what());
                return;
            }

            try {
                cursor.del();

                if (m_rebuild_keys) {
                    auto new_key = record->id.to_string();
                    m_db.put(m_transaction, new_key, *serialized);
                    fmt::print(fg(fmt::terminal_color::green), "{}: Updated with reworked key\n",
                               new_key);
                } else {
                    m_db.put(m_transaction, std::string(key), *serialized);
                    fmt::print(fg(fmt::terminal_color::green),
                               "{}: Updated without reworking key\n", key);
                }
            } catch (std::exception const& e) {
                handle_error("{}: Failed to update record: {}", record->id.to_string(), e.what());
            }
        }
    }

private:
    using Serializer = bxt::Utilities::LMDB::CerealSerializer<bxt::Persistence::Box::PackageRecord>;

    lmdb::txn& m_transaction;
    lmdb::dbi& m_db;
    bool m_rebuild_descfile = false;
    bool m_rebuild_keys = false;
    int m_error_count = 0;
};
