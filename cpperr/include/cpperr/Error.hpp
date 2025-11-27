/* === cpperr - Modern C++ Error Handling ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: MIT
 *
 */
#pragma once

#include <concepts>
#include <expected>
#include <source_location>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

namespace cpperr {

/**
 * Concept for error kinds that have a what() method (struct-based).
 */
template<typename T>
concept has_what_method = requires(T const& kind) {
    { kind.what() } -> std::convertible_to<std::string_view>;
};

/**
 * Concept for error kinds that have ADL to_string function (enum-based).
 */
template<typename T>
concept has_to_string = requires(T const& kind) {
    { to_string(kind) } -> std::convertible_to<std::string_view>;
};

/**
 * Unified error_kind concept - supports both patterns.
 */
template<typename T>
concept error_kind = has_what_method<T> || has_to_string<T>;

/**
 * Helper to get the string representation of an error kind.
 * Uses ADL to_string if available, otherwise calls what() method.
 */
template<error_kind T>
inline std::string_view error_kind_what(T const& kind) {
    if constexpr (has_what_method<T>) {
        return kind.what();
    } else {
        return to_string(kind);  // ADL lookup
    }
}

/**
 * Represents a single error in the cause chain.
 */
struct cause {
    std::type_info const* type;  // RTTI of the error kind
    std::string trace;           // Full trace of this cause
};

/**
 * Core error type wrapping a typed error kind with context and cause chain.
 * 
 * Stores error causes as a vector of type info + trace pairs for queryability.
 * 
 * @tparam TKind The error kind type (typically an enum)
 */
template<error_kind TKind>
class error {
public:
    using kind_type = TKind;  // Expose the kind type for metaprogramming
    
    /**
     * Construct an error with all components.
     */
    error(TKind kind, 
          std::string context = "",
          std::vector<cause> causes = {},
          std::source_location location = std::source_location::current())
        : m_kind(kind)
        , m_context(std::move(context))
        , m_causes(std::move(causes))
        , m_location(location)
    {}

    /**
     * Get the error kind.
     */
    TKind kind() const { return m_kind; }

    /**
     * Get the error context string.
     */
    std::string const& context() const { return m_context; }

    /**
     * Get the source location where this error was created.
     */
    std::source_location const& location() const { return m_location; }

    /**
     * Get the immediate error message (this error only, no causes).
     */
    std::string what() const {
        if (m_context.empty()) {
            return std::string(error_kind_what(m_kind));
        }
        return std::string(error_kind_what(m_kind)) + ": " + m_context;
    }

    /**
     * Get the full error trace including all causes.
     * Format:
     *   ErrorKind: context at file.cpp:123
     *   Caused by:
     *     LowerErrorKind: lower context at other.cpp:456
     */
    std::string trace() const {
        std::string result = what();
        result += " at ";
        result += m_location.file_name();
        result += ":";
        result += std::to_string(m_location.line());

        if (!m_causes.empty()) {
            result += "\nCaused by:";
            for (auto const& cause : m_causes) {
                result += "\n  ";
                // Indent cause traces
                for (char c : cause.trace) {
                    result += c;
                    if (c == '\n') {
                        result += "  ";
                    }
                }
            }
        }

        return result;
    }

    /**
     * Check if this error was caused by a specific error type.
     * Searches the entire cause chain for a matching type.
     * 
     * @tparam E The error kind type to check for
     * @return true if any cause in the chain is of type E
     */
    template<typename E>
    bool caused_by() const {
        for (auto const& cause : m_causes) {
            if (cause.type && *cause.type == typeid(E)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Get the full chain of error causes.
     * 
     * @return Const reference to the vector of causes
     * 
     * Example:
     *   for (auto const& cause : error.causes_chain()) {
     *       std::cout << cause.type->name() << ": " << cause.trace << '\n';
     *   }
     */
    std::vector<cause> const& causes_chain() const {
        return m_causes;
    }

private:
    TKind m_kind;                          // The actual error enum value
    std::string m_context;                 // Optional human-readable context
    std::vector<cause> m_causes;           // Chain of causes
    std::source_location m_location;       // Where this error was created
};

/**
 * Result type alias - either a value or an error.
 * 
 * @tparam T The success type
 * @tparam E The error kind type
 */
template<typename T, error_kind E>
using result = std::expected<T, error<E>>;

} // namespace cpperr
