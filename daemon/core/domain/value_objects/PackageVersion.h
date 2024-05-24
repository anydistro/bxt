/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/value_objects/Name.h"
#include "utilities/Error.h"

#include <expected>
#include <fmt/format.h>
#include <frozen/map.h>
#include <frozen/string.h>
#include <optional>
#include <string>

namespace bxt::Core::Domain {

struct PackageVersion {
    struct ParsingError : public bxt::Error {
        enum class ErrorCode {
            InvalidFormat,
            InvalidVersion,
            InvalidReleaseTag,
            InvalidEpoch
        };

        ErrorCode error_code;

        static inline const frozen::map<ErrorCode, frozen::string, 4>
            error_messages = {
                {ErrorCode::InvalidFormat, "Invalid format"},
                {ErrorCode::InvalidVersion, "Invalid version"},
                {ErrorCode::InvalidReleaseTag, "Invalid release tag"},
                {ErrorCode::InvalidEpoch, "Invalid epoch"}};

        ParsingError(ErrorCode error_code) : error_code(error_code) {
            message = error_messages.at(error_code).data();
        }
    };
    using ParseResult = std::expected<PackageVersion, ParsingError>;

    static std::strong_ordering compare(const PackageVersion& lh,
                                        const PackageVersion& rh);

    auto operator<=>(const PackageVersion& rh) const {
        return compare(*this, rh);
    };

    static ParseResult from_string(std::string_view str);
    std::string string() const;

    Name version;
    Name epoch;
    std::optional<Name> release = {};
};

} // namespace bxt::Core::Domain
