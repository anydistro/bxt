/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "utilities/Error.h"
#include "utilities/alpmdb/Desc.h"
#include "utilities/errors/DatabaseError.h"
#include "utilities/libarchive/Writer.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/log/trivial.hpp>
#include <coro/event.hpp>
#include <coro/io_scheduler.hpp>
#include <coro/mutex.hpp>
#include <coro/sync_wait.hpp>
#include <coro/task.hpp>
#include <expected>
#include <filesystem>
#include <fmt/format.h>
#include <frozen/map.h>
#include <frozen/set.h>
#include <frozen/string.h>
#include <iterator>
#include <memory>
#include <parallel_hashmap/phmap.h>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

namespace bxt::Utilities::AlpmDb::DatabaseUtils {

BXT_DECLARE_RESULT(DatabaseError)

coro::task<Result<phmap::parallel_flat_hash_map<std::string, Desc>>>
    parse_packages(std::set<std::string> files);

coro::task<Result<phmap::parallel_flat_hash_map<std::string, Desc>>>
    load(std::filesystem::path path);

coro::task<Result<void>>
    accept(std::set<std::string> files,
           std::function<void(const std::string& name, const Desc& description)>
               visitor);

template<typename TBuffer>
Result<void> write_buffer_to_archive(Archive::Writer& writer,
                                     const std::string& name,
                                     const TBuffer& buffer) {
    auto header = Archive::Header::default_file();

    archive_entry_set_pathname(header, name.c_str());
    archive_entry_set_size(header, buffer.size());

    auto entry = writer.start_write(header);

    if (!entry.has_value()) {
        return bxt::make_error_with_source<DatabaseError>(
            std::move(entry.error()), DatabaseError::ErrorType::IOError);
    }

    auto write_ok = entry->write({buffer.begin(), buffer.end()});

    if (!write_ok.has_value()) {
        return bxt::make_error_with_source<DatabaseError>(
            std::move(write_ok.error()), DatabaseError::ErrorType::IOError);
    }

    auto finish_ok = entry->finish();

    if (!finish_ok.has_value()) {
        return bxt::make_error_with_source<DatabaseError>(
            std::move(finish_ok.error()), DatabaseError::ErrorType::IOError);
    }

    return {};
}

coro::task<Result<void>>
    save(phmap::parallel_flat_hash_map<std::string, Desc> descriptions,
         std::filesystem::path path);

} // namespace bxt::Utilities::AlpmDb::DatabaseUtils
