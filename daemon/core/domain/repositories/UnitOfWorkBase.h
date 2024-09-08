/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "frozen/unordered_map.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <coro/sync_wait.hpp>
#include <coro/task.hpp>
#include <expected>

namespace bxt::Core::Domain {
struct UnitOfWorkBase {
    struct Error : public bxt::Error {
        enum class ErrorType { OperationError };

        Error(ErrorType error_type)
            : error_type(error_type) {
            message = error_messages.at(error_type).data();
        }

        ErrorType error_type;

    private:
        static inline frozen::unordered_map<ErrorType, std::string_view, 1> error_messages = {
            {ErrorType::OperationError, "Operation error"},
        };
    };
    BXT_DECLARE_RESULT(Error)

    virtual ~UnitOfWorkBase() {
    }

    virtual coro::task<Result<void>> commit_async() = 0;

    virtual coro::task<Result<void>> rollback_async() = 0;

    virtual coro::task<Result<void>> begin_async() = 0;

    virtual coro::task<Result<void>> begin_ro_async() = 0;

    virtual void hook(std::function<void()>&& hook, std::string const& name = "") = 0;
};

struct UnitOfWorkBaseFactory {
    virtual ~UnitOfWorkBaseFactory() = default;
    virtual coro::task<std::shared_ptr<UnitOfWorkBase>> operator()(bool rw = false) = 0;
};

} // namespace bxt::Core::Domain
