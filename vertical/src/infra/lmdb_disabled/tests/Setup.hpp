
#pragma once

#include <filesystem>
#include <memory>

#include <catch2/catch_test_macros.hpp>
#include <coro/io_scheduler.hpp>
#include <lmdbxx/lmdb++.h>

#include "../LmdbEnvironment.hpp"
#include "infra/logging/Logging.hpp"
#include "utils/MemoryLiterals.hpp"

namespace bxt::adapters::lmdb {

using namespace MemoryLiterals;

namespace TestSetup {
    static constexpr auto mapsize = 50_GiB;
    static constexpr auto max_dbs = 128;
    static constexpr auto mode = 0664;
    static constexpr auto Options = Environment::Options {.max_readers = 126,
                                                          .max_dbs = max_dbs,
                                                          .map_size = mapsize,
                                                          .mode = mode,
                                                          .flags = MDB_NOSUBDIR};

    inline std::pair<std::shared_ptr<Environment>, std::shared_ptr<coro::io_scheduler>>
        create_test_environment(std::string const& db_name) {
        auto scheduler = coro::io_scheduler::make_shared();
        auto temp_path = std::filesystem::temp_directory_path() / (db_name + ".mdb");

        std::error_code ec;
        if (std::filesystem::exists(temp_path)) {
            std::filesystem::remove(temp_path);
        }

        if (std::filesystem::create_directories(temp_path.parent_path(), ec); ec.value()) {
            logf("Cannot create LMDB folder. The error is \"{}\". Exiting.", ec.message());
            exit(1);
        }

        return std::make_pair(std::make_shared<Environment>(scheduler, temp_path.string(), Options),
                              scheduler);
    }

    inline void cleanup_environment(std::string const& db_name) {
        auto temp_path = std::filesystem::temp_directory_path() / (db_name + ".mdb");
        std::error_code ec;
        std::filesystem::remove_all(temp_path, ec);

        if (ec.value()) {
            logf("Cannot remove LMDB folder. The error is \"{}\". Exiting.", ec.message());
            exit(1);
        }
    }
} // namespace TestSetup
} // namespace bxt::adapters::lmdb
