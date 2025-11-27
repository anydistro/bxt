/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/formatter.h>
#include <spdlog/spdlog.h>

#include "FormattingUtils.hpp"

namespace bxt {

class FileFormatter : public spdlog::formatter {
public:
    std::unique_ptr<spdlog::formatter> clone() const override {
        return std::make_unique<FileFormatter>(*this);
    }

    void format(spdlog::details::log_msg const& msg, spdlog::memory_buf_t& dest) override {
        auto thread_id = std::this_thread::get_id();
        auto& ctx = m_thread_contexts[thread_id];

        // Calculate elapsed time

        std::string elapsed_time = logging::calculate_elapsed_time(ctx);

        // Format timestamp
        std::string timestamp = logging::format_timestamp(msg);

        // Format level
        std::string level_str = fmt::format("{:<7}", spdlog::level::to_string_view(msg.level));

        // Parse message to handle structured data
        auto [message, structured_data] = logging::parse_message(msg.payload);

        // Format source location

        std::string source = logging::format_source_location(msg.source);

        // Format the complete log entry
        std::string formatted =
            fmt::format("{} {} | {} | tid={:<5} | {}{}", timestamp, elapsed_time, level_str,
                        std::hash<std::thread::id> {}(std::this_thread::get_id()), message, source);
        // Add structured data if present
        if (!structured_data.empty()) {
            formatted += " | data=";
            for (size_t i = 0; i < structured_data.size(); ++i) {
                if (i > 0) {
                    formatted += ", ";
                }
                for (auto const& line : structured_data[i].lines) {
                    formatted += line;
                }
            }
        }

        // Add newline
        formatted += "\n";

        // Copy to destination buffer
        dest.append(formatted.data(), formatted.data() + formatted.size());
    }

private:
    std::unordered_map<std::thread::id, logging::ThreadTimeContext> m_thread_contexts;
};

} // namespace bxt
