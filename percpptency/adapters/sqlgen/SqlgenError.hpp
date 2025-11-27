/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <ranges>
#include <string>
#include <string_view>

#include <frozen/map.h>
#include <frozen/string.h>
#include <rfl/Result.hpp>

namespace percpptency::adapters::sqlgen {

class SqlgenError {
public:
    enum class Kind {
        InvalidSyntax,
        TableNotFound,
        ColumnNotFound,
        DuplicateKey,
        ForeignKeyViolation,
        DataTypeMismatch,
        NullConstraintViolation,
        UniqueConstraintViolation,
        ConnectionError,
        QueryTimeout,
        InvalidParameter,
        TransactionError,
        UnexpectedError
    };

    explicit SqlgenError(rfl::Error error)
        : m_reflect_error(std::move(error)) {
    }

    std::string_view what() const noexcept {
        return m_reflect_error.what();
    }

    [[nodiscard]] Kind kind() const noexcept {
        return sql_to_kind(m_reflect_error.what());
    }

private:
    static constexpr auto ErrorPatterns = frozen::make_map<frozen::string, Kind>({
        // SQLite syntax errors
        {"syntax error", Kind::InvalidSyntax},
        {"near \"", Kind::InvalidSyntax},
        {"malformed", Kind::InvalidSyntax},
        {"incomplete sql", Kind::InvalidSyntax},

        // SQLite table errors
        {"no such table", Kind::TableNotFound},
        {"table already exists", Kind::DuplicateKey},

        // SQLite column errors
        {"no such column", Kind::ColumnNotFound},
        {"duplicate column name", Kind::DuplicateKey},

        // SQLite constraint errors
        {"unique constraint failed", Kind::UniqueConstraintViolation},
        {"primary key constraint failed", Kind::DuplicateKey},
        {"foreign key constraint failed", Kind::ForeignKeyViolation},
        {"not null constraint failed", Kind::NullConstraintViolation},
        {"check constraint failed", Kind::InvalidParameter},

        // SQLite data type errors
        {"datatype mismatch", Kind::DataTypeMismatch},
        {"cannot convert", Kind::DataTypeMismatch},

        // SQLite connection/database errors
        {"database is locked", Kind::ConnectionError},
        {"database disk image is malformed", Kind::ConnectionError},
        {"unable to open database", Kind::ConnectionError},
        {"database or disk is full", Kind::ConnectionError},
        {"permission denied", Kind::ConnectionError},

        // SQLite transaction errors
        {"cannot start a transaction", Kind::TransactionError},
        {"cannot commit transaction", Kind::TransactionError},
        {"cannot rollback transaction", Kind::TransactionError},
        {"database schema has changed", Kind::TransactionError},

        // SQLite parameter errors
        {"column index out of range", Kind::InvalidParameter},
        {"bind parameter error", Kind::InvalidParameter},
        {"parameter count mismatch", Kind::InvalidParameter},

        // SQLite timeout
        {"database is busy", Kind::QueryTimeout},
        {"query timeout", Kind::QueryTimeout},
    });

    static Kind sql_to_kind(std::string const& sql) noexcept {
        // Convert to lowercase for case-insensitive matching
        auto lower_sql = sql | std::views::transform([](auto c) { return std::tolower(c); })
                         | std::ranges::to<std::string>();

        // Check each pattern in the map
        for (auto const& [pattern, kind] : ErrorPatterns) {
            if (lower_sql.contains(std::string_view {pattern.data(), pattern.size()})) {
                return kind;
            }
        }

        return Kind::UnexpectedError;
    }

    rfl::Error m_reflect_error;
};

} // namespace percpptency::adapters::sqlgen
