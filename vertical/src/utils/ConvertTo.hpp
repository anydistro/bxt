/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <cpperr/Factory.hpp>
#include "core/result_types.hpp"
#include "core/types/Path.hpp"
#include "core/types/TimePoint.hpp"
#include "core/types/to_string.hpp"
#include "core/error/kinds.hpp"

namespace bxt::utils {

enum class ConversionError {
    Failed,
    InvalidArgument,
    NotImplemented,
    OutOfRange,
    Overflow,
    Underflow
};

inline std::string_view to_string(ConversionError e) {
    switch (e) {
        case ConversionError::Failed: return "Failed";
        case ConversionError::InvalidArgument: return "InvalidArgument";
        case ConversionError::NotImplemented: return "NotImplemented";
        case ConversionError::OutOfRange: return "OutOfRange";
        case ConversionError::Overflow: return "Overflow";
        case ConversionError::Underflow: return "Underflow";
    }
    return "Unknown";
}

template<typename TTarget, typename TSource> struct convert_to {
    Result<TTarget, ConversionError> operator()(TSource const& value) const;
};

template<typename TTarget, typename TSource>
Result<TTarget, ConversionError>
    convert_to<TTarget, TSource>::operator()([[maybe_unused]] TSource const& value) const {
    return cpperr::make_error(ConversionError::NotImplemented);
}

template<typename TTarget, typename TSourceResult>
Result<TTarget, ConversionError> try_convert_to(TSourceResult const& source) {
    if (!source) {
        return cpperr::wrap_error(ConversionError::InvalidArgument, source.error());
    }

    auto converter = convert_to<TTarget, typename TSourceResult::value_type>();
    return converter(source.value());
}

} // namespace bxt::utils

// Add ConversionError to CrudError conversion in cpperr namespace
namespace cpperr {
template<>
inline bxt::err::CrudError convert_error<bxt::err::CrudError>(bxt::utils::ConversionError source) {
    using namespace bxt::err;
    using namespace bxt::utils;
    
    switch (source) {
        case ConversionError::InvalidArgument:
            return CrudError::InvalidData;
        case ConversionError::Failed:
        case ConversionError::NotImplemented:
            return CrudError::SerializationError;
        case ConversionError::OutOfRange:
        case ConversionError::Overflow:
        case ConversionError::Underflow:
            return CrudError::InvalidData;
    }
    return CrudError::SerializationError;
}
}
