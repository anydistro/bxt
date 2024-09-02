/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/domain/value_objects/PackageVersion.h"
#include "utilities/alpmdb/Desc.h"
#include "utilities/errors/Macro.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <filesystem>

namespace bxt::Core::Domain {
class PackagePoolEntry {
public:
    struct ParsingError : public bxt::Error {
        enum class ErrorCode {
            InvalidFilename,
            InvalidVersion,
            InvalidReleaseTag,
            InvalidName,
            InvalidEpoch,
            InvalidPackage
        };

        const ErrorCode error_code;

        static inline const frozen::map<ErrorCode, std::string_view, 6>
            error_messages = {
                {ErrorCode::InvalidFilename, "Invalid filename"},
                {ErrorCode::InvalidVersion, "Invalid package version"},
                {ErrorCode::InvalidName, "Invalid package name"},
                {ErrorCode::InvalidReleaseTag, "Invalid release tag"},
                {ErrorCode::InvalidEpoch, "Invalid epoch"},
                {ErrorCode::InvalidPackage, "Invalid package"}};

        ParsingError(ErrorCode error_code) : error_code(error_code) {
            message = error_messages.at(error_code).data();
        }
    };
    BXT_DECLARE_RESULT(ParsingError);

    PackagePoolEntry(std::filesystem::path m_file_path,
                     std::optional<std::filesystem::path> m_signature_path,
                     Utilities::AlpmDb::Desc desc,
                     PackageVersion m_version)
        : m_file_path(std::move(m_file_path)),
          m_signature_path(std::move(m_signature_path)),
          m_desc(std::move(desc)),
          m_version(std::move(m_version)) {}

    std::filesystem::path file_path() const { return m_file_path; }

    std::optional<std::filesystem::path> signature_path() const {
        return m_signature_path;
    }

    PackageVersion version() const { return m_version; }

    Utilities::AlpmDb::Desc desc() const { return m_desc; }

    static Result<PackagePoolEntry> parse_file_path(
        const std::filesystem::path& file_path,
        const std::optional<std::filesystem::path>& signature_path);

private:
    std::filesystem::path m_file_path;
    std::optional<std::filesystem::path> m_signature_path;
    Utilities::AlpmDb::Desc m_desc;

    PackageVersion m_version;
};
} // namespace bxt::Core::Domain
