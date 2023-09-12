/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: %YEAR% Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "frozen/unordered_map.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <coro/sync_wait.hpp>
#include <coro/task.hpp>
#include <functional>
#include <nonstd/expected.hpp>

namespace bxt::Core::Domain {
struct UnitOfWorkBase {
    struct Error : public bxt::Error {
        enum class ErrorType { OperationError };

        Error(ErrorType error_type, const bxt::Error&& result)
            : bxt::Error(std::make_unique<bxt::Error>(std::move(result))),
              error_type(error_type) {}

        ErrorType error_type;

        const std::string message() const noexcept override {
            return error_messages.at(error_type).data();
        }

    private:
        static inline frozen::unordered_map<ErrorType, std::string_view, 2>
            error_messages = {
                {ErrorType::OperationError, "Operation error"},
            };
    };
    BXT_DECLARE_RESULT(Error)

    virtual ~UnitOfWorkBase() {}

    virtual coro::task<Result<void>> commit_async() = 0;

    virtual coro::task<Result<void>> rollback_async() = 0;
};
} // namespace bxt::Core::Domain
