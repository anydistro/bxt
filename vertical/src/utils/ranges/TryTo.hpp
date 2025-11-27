/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <concepts>
#include <expected>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace bxt::ranges {

template<template<typename...> class TContainer = std::vector> struct try_to_t {
    template<std::ranges::range R>
        requires requires(std::ranges::range_value_t<R> result) {
            { result.has_value() } -> std::convertible_to<bool>;
            { result.value() };
            { result.error() };
        }
    constexpr auto operator()(R&& range) const {
        using ValueType =
            std::remove_cvref_t<decltype(std::declval<std::ranges::range_value_t<R>>().value())>;
        using ErrorType =
            std::remove_cvref_t<decltype(std::declval<std::ranges::range_value_t<R>>().error())>;
        using ContainerType = TContainer<ValueType>;
        using ResultType = std::expected<ContainerType, ErrorType>;

        ContainerType container;

        // Reserve space for containers that support it
        if constexpr (requires { container.reserve(std::size_t {}); }) {
            if constexpr (std::ranges::sized_range<R>) {
                container.reserve(std::ranges::size(range));
            }
        }

        // Collect values, stopping on first error
        for (auto&& result : range) {
            if (!result.has_value()) {
                return ResultType {std::unexpected(result.error())};
            }

            // Insert value into container
            if constexpr (requires { container.push_back(result.value()); }) {
                container.push_back(std::forward<decltype(result)>(result).value());
            } else if constexpr (requires { container.insert(result.value()); }) {
                container.insert(std::forward<decltype(result)>(result).value());
            } else {
                static_assert(false, "Container type not supported by try_to");
            }
        }

        return ResultType {std::move(container)};
    }
};

template<std::ranges::viewable_range R, template<typename...> class TContainer>
constexpr auto operator|(R&& range, try_to_t<TContainer> const& adapter) {
    return adapter(std::forward<R>(range));
}

template<template<typename...> class TContainer = std::vector>
inline constexpr try_to_t<TContainer> try_to {};

} // namespace bxt::ranges
