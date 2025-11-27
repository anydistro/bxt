/* === cpperr - Modern C++ Error Handling ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: MIT
 *
 */
#pragma once

#include "Error.hpp"
#include <concepts>
#include <source_location>
#include <typeinfo>
#include <expected>

namespace cpperr {

/**
 * Convert one error kind to another.
 * Users provide overloads of this function template for their error types.
 * 
 * Example:
 *   enum class LowLevel { NotFound, Failed };
 *   enum class HighLevel { Missing, Error };
 *   
 *   HighLevel convert_error(LowLevel source) {
 *       switch (source) {
 *           case LowLevel::NotFound: return HighLevel::Missing;
 *           default: return HighLevel::Error;
 *       }
 *   }
 */
template<typename TTarget, typename TSource>
TTarget convert_error(TSource source);

/**
 * Concept to check if an error conversion is defined.
 */
template<typename TTarget, typename TSource>
concept convertible_error = requires(TSource s) {
    { convert_error<TTarget>(s) } -> std::same_as<TTarget>;
};

/**
 * Create an error with just the kind (minimal form).
 * Source location is captured automatically.
 * 
 * @param kind The error kind enum value
 * @param loc Source location (automatically captured)
 * @return std::unexpected containing the error
 */
template<typename E>
auto make_error(E kind,
                std::source_location loc = std::source_location::current())
    -> std::unexpected<error<E>>
{
    return std::unexpected<error<E>>(error<E>(kind, "", {}, loc));
}

/**
 * Create an error with kind and context string.
 * Source location is captured automatically.
 * 
 * @param kind The error kind enum value
 * @param context Human-readable context string
 * @param loc Source location (automatically captured)
 * @return std::unexpected containing the error
 */
template<typename E>
auto make_error(E kind,
                std::string context,
                std::source_location loc = std::source_location::current())
    -> std::unexpected<error<E>>
{
    return std::unexpected<error<E>>(error<E>(kind, std::move(context), {}, loc));
}

/**
 * Wrap a lower-level error with a higher-level error kind.
 * Preserves the source error's trace in the cause chain.
 * 
 * @param target_kind The new (higher-level) error kind
 * @param source The original error to wrap
 * @param loc Source location (automatically captured)
 * @return std::unexpected containing the wrapped error
 */
template<typename TTarget, typename TSource>
auto wrap_error(TTarget target_kind,
                error<TSource> const& source,
                std::source_location loc = std::source_location::current())
    -> std::unexpected<error<TTarget>>
{
    std::vector<cause> new_causes;
    new_causes.push_back({&typeid(TSource), source.trace()});
    auto const& source_causes = source.causes_chain();
    new_causes.insert(new_causes.end(), source_causes.begin(), source_causes.end());
    
    return std::unexpected<error<TTarget>>(error<TTarget>(target_kind, "", std::move(new_causes), loc));
}

/**
 * Wrap a lower-level error with a higher-level error kind and context.
 * Preserves the source error's trace in the cause chain.
 * 
 * @param target_kind The new (higher-level) error kind
 * @param source The original error to wrap
 * @param context Human-readable context string
 * @param loc Source location (automatically captured)
 * @return std::unexpected containing the wrapped error
 */
template<typename TTarget, typename TSource>
auto wrap_error(TTarget target_kind,
                error<TSource> const& source,
                std::string context,
                std::source_location loc = std::source_location::current())
    -> std::unexpected<error<TTarget>>
{
    std::vector<cause> new_causes;
    new_causes.push_back({&typeid(TSource), source.trace()});
    auto const& source_causes = source.causes_chain();
    new_causes.insert(new_causes.end(), source_causes.begin(), source_causes.end());
    
    return std::unexpected<error<TTarget>>(error<TTarget>(target_kind, std::move(context), std::move(new_causes), loc));
}

/**
 * Auto-wrap an error using convert_error function.
 * Automatically converts source error kind to target kind.
 * Requires convert_error<TTarget>(TSource) to be defined.
 * 
 * @tparam TTarget The target error kind type
 * @param source The original error to wrap
 * @param loc Source location (automatically captured)
 * @return std::unexpected containing the converted error
 */
template<typename TTarget, typename TSource>
    requires convertible_error<TTarget, TSource>
auto wrap_error(error<TSource> const& source,
                std::source_location loc = std::source_location::current())
    -> std::unexpected<error<TTarget>>
{
    auto target_kind = convert_error<TTarget>(source.kind());
    return wrap_error(target_kind, source, loc);
}

/**
 * Auto-wrap an error with context using convert_error function.
 * Automatically converts source error kind to target kind.
 * Requires convert_error<TTarget>(TSource) to be defined.
 * 
 * @tparam TTarget The target error kind type
 * @param source The original error to wrap
 * @param context Human-readable context string
 * @param loc Source location (automatically captured)
 * @return std::unexpected containing the converted error
 */
template<typename TTarget, typename TSource>
    requires convertible_error<TTarget, TSource>
auto wrap_error(error<TSource> const& source,
                std::string context,
                std::source_location loc = std::source_location::current())
    -> std::unexpected<error<TTarget>>
{
    auto target_kind = convert_error<TTarget>(source.kind());
    return wrap_error(target_kind, source, std::move(context), loc);
}

} // namespace cpperr
