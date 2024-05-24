/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include <filesystem>
#include <rfl/parsing/CustomParser.hpp>
#include <string>

struct Path {
    std::string path;

    static Path from_class(const std::filesystem::path& _p) noexcept {
        return Path {_p.string()};
    }
    std::filesystem::path to_class() const {
        return std::filesystem::path {path};
    }
};
namespace rfl::parsing {

template<class ReaderType, class WriterType, class ProcessorsType>
struct Parser<ReaderType, WriterType, std::filesystem::path, ProcessorsType>
    : public CustomParser<ReaderType,
                          WriterType,
                          ProcessorsType,
                          std::filesystem::path,
                          Path> {};

} // namespace rfl::parsing
