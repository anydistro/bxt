/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageDTO.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <coro/task.hpp>
#include <filesystem>
#include <frozen/unordered_map.h>

namespace bxt::Core::Application {

class DeploymentService {
public:
    struct Error : public bxt::Error {
        enum class ErrorType {
            InvalidSession,
            PackagePushFailed,
            DeploymentFailed,
            InvalidArgument
        };

        Error(ErrorType error_type) : error_type(error_type) {
            message = error_messages.at(error_type).data();
        }

        ErrorType error_type;

    private:
        static inline frozen::unordered_map<ErrorType, std::string, 4>
            error_messages = {
                {ErrorType::InvalidSession, "Invalid session."},
                {ErrorType::PackagePushFailed, "Package push failed."},
                {ErrorType::DeploymentFailed, "Deployment failed."},
                {ErrorType::InvalidArgument, "Invalid argument."}};
    };
    BXT_DECLARE_RESULT(Error)

    virtual ~DeploymentService() = default;

    virtual coro::task<Result<uint64_t>> deploy_start() = 0;
    virtual coro::task<Result<void>> deploy_push(PackageDTO package,
                                                 uint64_t session_id) = 0;
    virtual coro::task<Result<void>> deploy_end(uint64_t session_id) = 0;

    virtual coro::task<Result<void>> verify_session(uint64_t session_id) = 0;
};

} // namespace bxt::Core::Application
