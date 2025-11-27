/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#include <catch2/catch_test_macros.hpp>

#include "infra/logging/FileFormatter.hpp"

TEST_CASE("FileFormatter formats log messages correctly", "[logging]") {
    bxt::FileFormatter formatter;
    spdlog::memory_buf_t dest;

    SECTION("Basic message formatting") {
        spdlog::details::log_msg msg;
        msg.level = spdlog::level::info;
        msg.payload = "Test message";

        formatter.format(msg, dest);
        std::string result {dest.data(), dest.size()};

        REQUIRE(result.find("Test message") != std::string::npos);
        REQUIRE(result.find("info") != std::string::npos);
    }

    SECTION("Structured data formatting") {
        spdlog::details::log_msg msg;
        msg.level = spdlog::level::debug;
        msg.payload = fmt::format("Message{}{}", bxt::logging::StructDataSeparator, "key=value");

        formatter.format(msg, dest);
        std::string result {dest.data(), dest.size()};

        REQUIRE(result.find("Message") != std::string::npos);
        REQUIRE(result.find("data=key=value") != std::string::npos);
    }

    SECTION("Source location formatting") {
        spdlog::details::log_msg msg;
        msg.level = spdlog::level::warn;
        msg.source.filename = "test.cpp";
        msg.source.line = 42;
        msg.source.funcname = "test_func";
        msg.payload = "Location test";

        formatter.format(msg, dest);
        std::string result {dest.data(), dest.size()};

        REQUIRE(result.find("[test.cpp:42]") != std::string::npos);
        REQUIRE(result.find("[test_func]") != std::string::npos);
    }

    SECTION("Elapsed time formatting") {
        spdlog::details::log_msg msg;
        msg.level = spdlog::level::info;
        msg.payload = "Time test";

        // First message
        formatter.format(msg, dest);
        std::string result1 {dest.data(), dest.size()};
        dest.clear();

        // Add delay and send second message
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        formatter.format(msg, dest);
        std::string result2 {dest.data(), dest.size()};

        REQUIRE(result2.find("+") != std::string::npos);
        REQUIRE(result2.find("ms") != std::string::npos);
    }

    SECTION("Thread ID formatting") {
        spdlog::details::log_msg msg;
        msg.level = spdlog::level::info;
        msg.payload = "Thread test";

        formatter.format(msg, dest);
        std::string result {dest.data(), dest.size()};

        REQUIRE(result.find("tid=") != std::string::npos);
    }

    SECTION("Multiple structured data items") {
        spdlog::details::log_msg msg;
        msg.level = spdlog::level::info;
        std::string payload =
            fmt::format("Message{}data1{}data2{}data3", bxt::logging::StructDataSeparator,
                        bxt::logging::StructDataSeparator, bxt::logging::StructDataSeparator);
        msg.payload = payload;

        formatter.format(msg, dest);
        std::string result {dest.data(), dest.size()};

        REQUIRE(result.find("Message") != std::string::npos);
        REQUIRE(result.find("data=data1, data2, data3") != std::string::npos);
    }
}
