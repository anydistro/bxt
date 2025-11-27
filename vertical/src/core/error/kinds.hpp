/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include <cpperr/Error.hpp>
#include <system_error>
#include <string>
#include <string_view>

namespace bxt::err {

/**
 * @brief Generic CRUD operation error kinds
 *
 * These represent fundamental failure modes in any storage system:
 *
 * @param NotFound - Entity doesn't exist (Read, Update, Delete operations)
 * @param AlreadyExists - Entity already exists (Create operations with uniqueness constraints)
 * @param InvalidData - Data validation failed (format, type, business rules)
 * @param ConstraintViolation - Data integrity or business constraints violated
 * @param PermissionDenied - Access control prevents the operation
 * @param ConcurrencyConflict - Concurrent operations conflict (optimistic locking, etc.)
 * @param StorageError - Underlying storage system failure
 * @param SerializationError - Failed to convert entity to storage format
 * @param DeserializationError - Failed to convert storage data back to entity
 */
enum class CrudError {
    NotFound,
    AlreadyExists,
    InvalidData,
    ConstraintViolation,
    PermissionDenied,
    ConcurrencyConflict,
    StorageError,
    SerializationError,
    DeserializationError
};

inline std::string_view to_string(CrudError e) {
    switch (e) {
        case CrudError::NotFound: return "NotFound";
        case CrudError::AlreadyExists: return "AlreadyExists";
        case CrudError::InvalidData: return "InvalidData";
        case CrudError::ConstraintViolation: return "ConstraintViolation";
        case CrudError::PermissionDenied: return "PermissionDenied";
        case CrudError::ConcurrencyConflict: return "ConcurrencyConflict";
        case CrudError::StorageError: return "StorageError";
        case CrudError::SerializationError: return "SerializationError";
        case CrudError::DeserializationError: return "DeserializationError";
    }
    return "Unknown";
}

enum class ParseError {
    InvalidFormat,
    MissingField,
    InvalidValue,
    UnsupportedVersion
};

inline std::string_view to_string(ParseError e) {
    switch (e) {
        case ParseError::InvalidFormat: return "InvalidFormat";
        case ParseError::MissingField: return "MissingField";
        case ParseError::InvalidValue: return "InvalidValue";
        case ParseError::UnsupportedVersion: return "UnsupportedVersion";
    }
    return "Unknown";
}

// Wrapper for standard system errors (std::filesystem and other)
struct SystemError {
    std::error_code code;
    std::string message;

    SystemError(std::error_code const& ec)
        : code(ec)
        , message(ec.message()) {
    }

    SystemError(std::system_error const& e)
        : code(e.code())
        , message(e.what()) {
    }

    std::string_view what() const {
        return message;
    }
};

} // namespace bxt::err

