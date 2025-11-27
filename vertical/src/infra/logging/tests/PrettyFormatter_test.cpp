/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#include <catch2/catch_test_macros.hpp>
#include <spdlog/details/log_msg.h>

#include "infra/logging/PrettyFormatter.hpp"

TEST_CASE("PrettyFormatter formats log messages correctly", "[logging]") {
    constexpr spdlog::source_loc TestSource = {"test.cpp", 42, "test_func"};

    bxt::PrettyFormatter formatter;
    spdlog::memory_buf_t dest;

    SECTION("Empty message") {
        spdlog::details::log_msg msg {TestSource, "test_logger", spdlog::level::info, ""};

        formatter.format(msg, dest);
        std::string result = fmt::to_string(dest);

        REQUIRE(result.find("(No message)") != std::string::npos);
        REQUIRE(result.find("test.cpp:42") != std::string::npos);
    }

    SECTION("Single line message") {
        spdlog::details::log_msg msg {TestSource, "test_logger", spdlog::level::info,
                                      "Test message"};

        formatter.format(msg, dest);
        std::string result = fmt::to_string(dest);

        REQUIRE(result.find("Test message") != std::string::npos);
        REQUIRE(result.find("test.cpp:42") != std::string::npos);
    }

    SECTION("Multi-line message") {
        spdlog::details::log_msg msg {TestSource, "test_logger", spdlog::level::info,
                                      "Line 1\nLine 2\nLine 3"};

        formatter.format(msg, dest);
        std::string result = fmt::to_string(dest);

        REQUIRE(result.find("Line 1") != std::string::npos);
        REQUIRE(result.find("Line 2") != std::string::npos);
        REQUIRE(result.find("Line 3") != std::string::npos);
        REQUIRE(result.find("test.cpp:42") != std::string::npos);
    }

    SECTION("Message with structured data") {
        spdlog::details::log_msg msg {TestSource, "test_logger", spdlog::level::info,
                                      "Message with data\n[data key=value]\n[another key=value2]"};

        formatter.format(msg, dest);
        std::string result = fmt::to_string(dest);

        REQUIRE(result.find("Message with data") != std::string::npos);
        REQUIRE(result.find("key=value") != std::string::npos);
        REQUIRE(result.find("key=value2") != std::string::npos);
    }

    SECTION("Different log levels") {
        auto test_level = [&](spdlog::level::level_enum level) {
            dest.clear();
            spdlog::details::log_msg msg {TestSource, "test_logger", level, "Test message"};
            formatter.format(msg, dest);
            std::string result = fmt::to_string(dest);
            REQUIRE(result.contains(std::string_view {spdlog::level::to_string_view(level)}));
        };

        test_level(spdlog::level::trace);
        test_level(spdlog::level::debug);
        test_level(spdlog::level::info);
        test_level(spdlog::level::warn);
        test_level(spdlog::level::err);
        test_level(spdlog::level::critical);
    }

    SECTION("Timestamp threshold") {
        spdlog::details::log_msg msg {TestSource, "test_logger", spdlog::level::info,
                                      "Test message"};

        // First message
        formatter.format(msg, dest);
        std::string first_result = fmt::to_string(dest);

        // Wait for threshold
        std::this_thread::sleep_for(bxt::PrettyFormatter::full_timestamp_threshold);

        // Second message
        dest.clear();
        formatter.format(msg, dest);
        std::string second_result = fmt::to_string(dest);

        // Second message should have full timestamp
        REQUIRE(second_result.length() > first_result.length());
    }
}
