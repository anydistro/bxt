/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "frozen/unordered_map.h"
#include "utilities/alpmdb/PkgInfo.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <cereal/access.hpp>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bxt::Utilities::AlpmDb {

struct Desc {
    struct ParseError : public bxt::Error {
        enum class ErrorType {
            InvalidArchive,
            NoPackageInfo,
        };

        ParseError(ErrorType type)
            : error_type(type) {
        }
        explicit ParseError(ErrorType type, bxt::Error const&& source)
            : bxt::Error(std::make_unique<bxt::Error>(std::move(source)))
            , error_type(type) {
            message = error_messages.at(error_type).data();
        }

    private:
        ErrorType error_type;

        // Define the error messages map
        static constexpr frozen::unordered_map<ErrorType, std::string_view, 2> error_messages = {
            {ErrorType::InvalidArchive, "This file is not a valid archive"},
            {ErrorType::NoPackageInfo, "No package info"},
        };
    };
    BXT_DECLARE_RESULT(ParseError)

    template<class Archive> void serialize(Archive& ar) {
        ar(desc, files);
    }

    static Result<Desc> parse_package(std::filesystem::path const& filepath,
                                      std::string const& signature = "",
                                      bool create_files = true);

    std::optional<std::string> get(std::string const& key) const;

    std::string desc;
    std::string files;
};
} // namespace bxt::Utilities::AlpmDb
