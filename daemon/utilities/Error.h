/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "fmt/format.h"

#include <expected>
#include <memory>
#include <string>

namespace bxt {

struct Error {
    Error() = default;
    explicit Error(std::unique_ptr<bxt::Error>&& source)
        : source(std::move(source)) {}

    Error(Error&& other) noexcept {
        source = std::move(other.source);
        message = std::move(other.message);
    }

    Error(const Error& other) {
        if (other.source) {
            source = std::make_unique<bxt::Error>(*other.source);
        }
        message = other.message;
    }
    Error& operator=(const Error& other) {
        if (this != &other) {
            if (other.source) {
                source = std::make_unique<bxt::Error>(*other.source);
            } else {
                source.reset();
            }
            message = other.message;
        }
        return *this;
    }
    Error& operator=(Error&& other) noexcept {
        if (this != &other) {
            source = std::move(other.source);
            message = std::move(other.message);
        }
        return *this;
    }

    virtual ~Error() = default;

    std::unique_ptr<bxt::Error> source = nullptr;

    const std::string what() const noexcept {
        auto result = message;
        if (source) { result += fmt::format("\nFrom:\n{}\n", source->what()); }
        return result;
    }

    std::string message = "Unknown error";
};

template<typename TError, typename TSource, typename... TArgs>
std::unexpected<TError> make_error_with_source(TSource&& source,
                                               TArgs... ctorargs) {
    TError result(ctorargs...);

    result.source = std::make_unique<TSource>(std::move(source));

    return std::unexpected(result);
}

template<typename TError, typename... TArgs>
std::unexpected<TError> make_error(TArgs... ctorargs) {
    TError result(ctorargs...);

    return std::unexpected(result);
}

} // namespace bxt
