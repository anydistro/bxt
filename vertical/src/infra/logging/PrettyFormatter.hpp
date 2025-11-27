/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/formatter.h>
#include <spdlog/spdlog.h>

#include "infra/logging/FormattingUtils.hpp"

namespace bxt {

class PrettyFormatter : public spdlog::formatter {
public:
    PrettyFormatter(std::string prefix = "")
        : m_prefix(std::move(prefix)) {
    }

    // Threshold in seconds for showing full timestamps
    static constexpr std::chrono::seconds full_timestamp_threshold {10};

    std::unique_ptr<spdlog::formatter> clone() const override {
        return std::make_unique<PrettyFormatter>(*this);
    }

    void format(spdlog::details::log_msg const& msg, spdlog::memory_buf_t& dest) override {
        // Per-thread timing context
        static thread_local logging::ThreadTimeContext ctx;

        // Prepare formatting context
        FormattingContext fmt_ctx;
        fmt_ctx.level = msg.level;

        // Get the full timestamp
        fmt_ctx.full_timestamp = logging::format_timestamp(msg);

        // Check if we should show full timestamp based on elapsed time
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - ctx.last_log_time);
        fmt_ctx.show_full_timestamp = elapsed >= full_timestamp_threshold;

        // Calculate and format elapsed time
        std::string plain_elapsed_time = logging::calculate_elapsed_time(ctx);
        fmt_ctx.elapsed_time = fmt::format(m_elapsed_time_color, "{}", plain_elapsed_time);
        fmt_ctx.level_str = format_level_string(msg.level);
        fmt_ctx.location = format_location(msg.source, msg.level);
        fmt_ctx.function = msg.source.funcname;

        std::tie(fmt_ctx.main_message, fmt_ctx.structured_data) =
            logging::parse_message(msg.payload);
        boost::split(fmt_ctx.lines, fmt_ctx.main_message, boost::is_any_of("\n"));

        try {
            std::string formatted = format_output(fmt_ctx);
            formatted.push_back('\n');
            dest.append(formatted.data(), formatted.data() + formatted.size());
        } catch (...) {
            // Fallback: raw message
            auto raw = fmt::format("{}\n", fmt::string_view {msg.payload});
            dest.append(raw.data(), raw.data() + raw.size());
        }

        // Reset last_log_time for next call
        ctx.last_log_time = now;
    }

private:
    struct FormattingContext {
        spdlog::level::level_enum level = spdlog::level::trace;
        std::string elapsed_time;
        std::string full_timestamp;
        bool show_full_timestamp = false;
        std::string level_str;
        std::string location;
        std::string function;
        std::string main_message;
        std::vector<logging::StructuredDataItem> structured_data;
        std::vector<std::string> lines;
    };

    // Style definitions
    std::unordered_map<spdlog::level::level_enum, fmt::text_style> m_level_colors = {

        {spdlog::level::trace, fg(fmt::color::light_gray)},
        {spdlog::level::debug, fg(fmt::color::cyan)},
        {spdlog::level::info, fg(fmt::color::lime_green)},
        {spdlog::level::warn, fg(fmt::color::gold)},
        {spdlog::level::err, fg(fmt::color::crimson)},
        {spdlog::level::critical, fg(fmt::color::dark_red)},
        {spdlog::level::off, fg(fmt::color::gainsboro)}

    };

    fmt::text_style m_elapsed_time_color = fg(fmt::color::dark_gray) | fmt::emphasis::italic;
    fmt::text_style m_line_number_color = fg(fmt::color::dark_gray);
    fmt::text_style m_struct_data_color = fg(fmt::color::white_smoke);
    fmt::text_style m_timestamp_color = fg(fmt::color::dark_cyan) | fmt::emphasis::italic;

    bool m_show_function_name = false;

    std::string m_prefix = "";

    // Style helper methods
    fmt::text_style location_color_(spdlog::level::level_enum level) const {
        auto style = m_level_colors.at(level);
        return style.has_emphasis() ? fmt::fg(style.get_foreground()) | fmt::emphasis::italic
                                    : style | fmt::emphasis::italic;
    }

    fmt::text_style function_color_(spdlog::level::level_enum level) const {
        return m_level_colors.at(level) | fmt::emphasis::italic;
    }

    fmt::text_style tree_color_(spdlog::level::level_enum level) const {
        return m_level_colors.at(level);
    }

    std::string format_level_string(spdlog::level::level_enum level) const {
        return fmt::format(m_level_colors.at(level), "{:<7}", spdlog::level::to_string_view(level));
    }

    std::string format_location(spdlog::source_loc const& source,
                                spdlog::level::level_enum level) const {
        return fmt::format(location_color_(level), "{}:{}", source.filename, source.line);
    }

    std::string format_function_name(FormattingContext const& ctx) const {
        if (ctx.function.empty()) {
            return "";
        }
        return fmt::format("[{}]", fmt::format(function_color_(ctx.level), "{}", ctx.function));
    }

    std::string format_tree_char(spdlog::level::level_enum level, char const* tree_char) const {
        return fmt::format(tree_color_(level), "{}", tree_char);
    }

    std::string format_output(FormattingContext const& ctx) {
        if (ctx.lines.empty() || (ctx.lines.size() == 1 && ctx.lines[0].empty())) {
            return format_empty_message(ctx);
        } else if (ctx.lines.size() == 1 && ctx.structured_data.empty()) {
            return format_single_line(ctx);
        } else {
            return format_multi_line(ctx);
        }
    }

    std::string format_empty_message(FormattingContext const& ctx) {
        // Header
        std::string formatted = build_header(ctx, "(No message)") + "\n";

        // Footer location
        if (!ctx.location.empty()) {
            formatted += build_footer_location(ctx, "└") + "\n";
        }

        return formatted;
    }

    std::string format_single_line(FormattingContext const& ctx) {
        // Header with the single line message
        std::string formatted = build_header(ctx, ctx.lines[0]) + "\n";

        // Footer location
        if (!ctx.location.empty()) {
            formatted += build_footer_location(ctx, "└") + "\n";
        }

        return formatted;
    }

    std::string format_multi_line(FormattingContext const& ctx) {
        std::string formatted;
        if (ctx.show_full_timestamp) {
            formatted = fmt::format(
                "{level} {tree} {msg} {elapsed} {timestamp}\n", fmt::arg("level", ctx.level_str),
                fmt::arg("tree", format_tree_char(ctx.level, "┌")),
                fmt::arg("msg", fmt::format(m_level_colors.at(ctx.level), "{}", ctx.lines[0])),
                fmt::arg("elapsed", ctx.elapsed_time),
                fmt::arg("timestamp", fmt::format(m_timestamp_color, "[{}]", ctx.full_timestamp)));
        } else {
            formatted = fmt::format(
                "{level} {tree} {msg} {elapsed}\n", fmt::arg("level", ctx.level_str),
                fmt::arg("tree", format_tree_char(ctx.level, "┌")),
                fmt::arg("msg", fmt::format(m_level_colors.at(ctx.level), "{}", ctx.lines[0])),
                fmt::arg("elapsed", ctx.elapsed_time));
        }

        // Location line
        if (!ctx.location.empty()) {
            bool is_simple_case = ctx.structured_data.empty() && ctx.lines.size() == 1;
            auto tree_char = is_simple_case ? "└" : "│";

            formatted += fmt::format(
                "{level} {tree} at {loc} {func}\n", fmt::arg("level", ctx.level_str),
                fmt::arg("tree", format_tree_char(ctx.level, tree_char)),
                fmt::arg("loc", ctx.location),
                fmt::arg("func", m_show_function_name ? format_function_name(ctx) : ""));
        }

        // Additional message lines
        for (size_t i = 1; i < ctx.lines.size(); ++i) {
            bool is_last_line = (i == ctx.lines.size() - 1 && ctx.structured_data.empty());
            auto tree_char = is_last_line ? "└" : "│";

            formatted += fmt::format(
                "{level} {tree} {line_num} {space} {msg}\n", fmt::arg("level", ctx.level_str),
                fmt::arg("tree", format_tree_char(ctx.level, tree_char)),
                fmt::arg("line_num", fmt::format(m_line_number_color, "[{:2}]", i)),
                fmt::arg("space", " "),
                fmt::arg("msg", fmt::format(m_level_colors.at(ctx.level), "{}", ctx.lines[i])));
        }

        // Calculate total line count for structured data items
        size_t total_structured_lines = 0;
        for (auto const& item : ctx.structured_data) {
            total_structured_lines += item.lines.size();
        }

        // Structured data lines
        size_t struct_line_count = 0;
        for (size_t i = 0; i < ctx.structured_data.size(); ++i) {
            auto const& item = ctx.structured_data[i];

            for (auto const& line : item.lines) {
                struct_line_count++;
                bool is_last_line = (struct_line_count == total_structured_lines);
                auto tree_char = is_last_line ? "└" : "│";

                formatted += fmt::format(
                    "{level} {tree} {line_num} {space} {msg}\n", fmt::arg("level", ctx.level_str),
                    fmt::arg("tree", format_tree_char(ctx.level, tree_char)),
                    fmt::arg("line_num",
                             fmt::format(m_line_number_color, "[{:2}]", struct_line_count)),
                    fmt::arg("space", " "),
                    fmt::arg("msg", fmt::format(m_struct_data_color, "{}", line)));
            }
        }

        return formatted;
    }

    // Helper to build the top ("┌") line for any message
    std::string build_header(FormattingContext const& ctx, std::string const& msg) const {
        if (ctx.show_full_timestamp) {
            return fmt::format(
                "{level} {tree} {prefix}{msg} {elapsed} {timestamp}",
                fmt::arg("level", ctx.level_str),
                fmt::arg("tree", format_tree_char(ctx.level, "┌")), fmt::arg("prefix", m_prefix),
                fmt::arg("msg", fmt::format(m_level_colors.at(ctx.level), "{}", msg)),
                fmt::arg("elapsed", ctx.elapsed_time),
                fmt::arg("timestamp", fmt::format(m_timestamp_color, "[{}]", ctx.full_timestamp)));
        }
        return fmt::format(
            "{level} {tree} {prefix}{msg} {elapsed}", fmt::arg("level", ctx.level_str),
            fmt::arg("tree", format_tree_char(ctx.level, "┌")), fmt::arg("prefix", m_prefix),
            fmt::arg("msg", fmt::format(m_level_colors.at(ctx.level), "{}", msg)),
            fmt::arg("elapsed", ctx.elapsed_time));
    }

    // Helper to build the bottom/location ("└" or "│") line
    std::string build_footer_location(FormattingContext const& ctx, char const* tree_char) const {
        return fmt::format("{level} {tree} at {loc} {func}", fmt::arg("level", ctx.level_str),
                           fmt::arg("tree", format_tree_char(ctx.level, tree_char)),
                           fmt::arg("loc", ctx.location),
                           fmt::arg("func", m_show_function_name ? format_function_name(ctx) : ""));
    }
};

} // namespace bxt
