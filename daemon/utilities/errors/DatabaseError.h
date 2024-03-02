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
        InvalidArgument
    };
    DatabaseError(ErrorType error_type) : error_type(error_type) {
        message = messages.at(error_type).data();
    }

    DatabaseError(ErrorType error_type, const bxt::Error&& source)
        : bxt::Error(std::make_unique<bxt::Error>(std::move(source))),
          error_type(error_type) {
        message = messages.at(error_type).data();
    }

    static inline frozen::map<ErrorType, std::string_view, 5> messages {
        {ErrorType::IOError, "IO error"},
        {ErrorType::DatabaseMalformedError, "Database is malformed"},
        {ErrorType::InvalidEntityError, "Invalid entity"},
        {ErrorType::EntityNotFound, "Entity not found"},
        {ErrorType::InvalidArgument, "Invalid argument"}};

private:
    ErrorType error_type;
};

} // namespace bxt
