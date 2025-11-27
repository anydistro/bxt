/* === This file is part of percpptency ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <concepts>
#include <rfl.hpp>

namespace percpptency {

// Concept: Aggregate with Data nested type following the percpptency pattern
template<typename T>
concept AggregateWithData = requires {
    typename T::Data;
};

// Check if aggregate has create_from_data static method (for validated construction)
template<typename T>
concept HasCreateFromData = requires(typename T::Data const& data) {
    { T::create_from_data(data) } -> std::same_as<T>;
};

// Check if aggregate is directly constructible from Data (simple construction)
template<typename T>
concept ConstructibleFromData = requires(typename T::Data const& data) {
    { T(data) } -> std::same_as<T>;
};

} // namespace percpptency

// Generic Reflector for aggregates following the pattern:
// - Has nested struct Data { ... }
// - Either has explicit constructor from Data OR static create_from_data(Data const&)
// - Inherits from AggregateRoot<T> (which has protected m_data field)
//
// For simple aggregates: explicit Aggregate(Data data)
// For validated aggregates: static Aggregate create_from_data(Data const& data)
namespace rfl {

template<percpptency::AggregateWithData T>
    requires (percpptency::HasCreateFromData<T> || percpptency::ConstructibleFromData<T>)
struct Reflector<T> {
    using ReflType = typename T::Data;
    
    // Construct aggregate from persistence data
    static T to_class(ReflType const& v) noexcept(false) {
        if constexpr (percpptency::HasCreateFromData<T>) {
            // Use factory method if available (for validation)
            return T::create_from_data(v);
        } else {
            // Direct construction for simple aggregates
            return T(v);
        }
    }
    
    // Extract persistence data from aggregate
    static ReflType from_class(T const& entity) noexcept {
        // Copy m_data (works for types with proper copy semantics)
        // AggregateRoot itself isn't copied (events are separate)
        return entity.m_data;
    }
};

} // namespace rfl
