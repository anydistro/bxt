/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "frozen/map.h"
#include "utilities/Error.h"

namespace bxt {

class DatabaseError : public bxt::Error {
public:
    enum class ErrorType {
        IOError,
        DatabaseMalformedError,
        InvalidEntityError,
        EntityNotFound,
        AlreadyExists,
        InvalidArgument
    };
    DatabaseError(ErrorType error_type)
        : error_type(error_type) {
        message = messages.at(error_type).data();
    }

    DatabaseError(ErrorType error_type, bxt::Error const&& source)
        : bxt::Error(std::make_unique<bxt::Error>(std::move(source)))
        , error_type(error_type) {
        message = messages.at(error_type).data();
    }

    static inline frozen::map<ErrorType, std::string_view, 6> messages {
        {ErrorType::IOError, "IO error"},
        {ErrorType::DatabaseMalformedError, "Database is malformed"},
        {ErrorType::InvalidEntityError, "Invalid entity"},
        {ErrorType::EntityNotFound, "Entity not found"},
        {ErrorType::AlreadyExists, "Entity already exists"},
        {ErrorType::InvalidArgument, "Invalid argument"}};

private:
    ErrorType error_type;
};

} // namespace bxt
