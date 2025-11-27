/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <cpperr/Factory.hpp>
#include "kinds.hpp"
#include "core/domain/repository_errors.hpp"

namespace bxt::utils {
// Forward declaration
enum class ConversionError;
}

namespace cpperr {

// CrudError → ParseError conversion
template<>
inline bxt::err::ParseError convert_error<bxt::err::ParseError>(bxt::err::CrudError source) {
    using namespace bxt::err;
    switch (source) {
        case CrudError::InvalidData:
            return ParseError::InvalidFormat;
        case CrudError::NotFound:
            return ParseError::MissingField;
        case CrudError::DeserializationError:
            return ParseError::InvalidValue;
        default:
            return ParseError::InvalidValue;
    }
}

// ParseError → CrudError conversion
template<>
inline bxt::err::CrudError convert_error<bxt::err::CrudError>(bxt::err::ParseError source) {
    using namespace bxt::err;
    switch (source) {
        case ParseError::InvalidFormat:
        case ParseError::InvalidValue:
            return CrudError::InvalidData;
        case ParseError::MissingField:
            return CrudError::NotFound;
        case ParseError::UnsupportedVersion:
            return CrudError::InvalidData;
    }
    return CrudError::InvalidData;
}

// TransactionError → CrudError conversion
template<>
inline bxt::err::CrudError convert_error<bxt::err::CrudError>(bxt::domain::repo::TransactionError source) {
    using namespace bxt::err;
    using namespace bxt::domain::repo;
    
    switch (source) {
        case TransactionError::BeginFailed:
        case TransactionError::ConnectionError:
        case TransactionError::InvalidState:
            return CrudError::StorageError;
        case TransactionError::ConcurrencyConflict:
            return CrudError::ConcurrencyConflict;
        case TransactionError::CommitFailed:
        case TransactionError::RollbackFailed:
            return CrudError::StorageError;
        case TransactionError::Timeout:
        case TransactionError::LockAcquisitionFailed:
            return CrudError::ConcurrencyConflict;
    }
    return CrudError::StorageError;
}

// ConversionError → CrudError conversion
template<>
inline bxt::err::CrudError convert_error<bxt::err::CrudError>(bxt::utils::ConversionError source);

} // namespace cpperr
