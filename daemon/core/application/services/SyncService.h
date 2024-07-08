/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/RequestContext.h"
#include "core/application/dtos/PackageSectionDTO.h"
#include "utilities/Error.h"
#include "utilities/errors/Macro.h"

#include <coro/task.hpp>
#include <frozen/string.h>
#include <frozen/unordered_map.h>

namespace bxt::Core::Application {

class SyncService {
public:
    struct SyncError : public bxt::Error {
        enum Type {
            NetworkError,
            IOError,
            ParseError,
            RepositoryError,
            ValidationError
        };
        Type error_type;

        SyncError(Type type) : error_type(type) {
            message = type_messages.at(type).data();
        }

        static constexpr frozen::unordered_map<Type, frozen::string, 5>
            type_messages {{NetworkError, "Network error occurred"},
                           {IOError, "Input/output error occurred"},
                           {ParseError, "Parsing error occurred"},
                           {RepositoryError, "Repository error occurred"},
                           {ValidationError, "Validation error occurred"}};
    };
    BXT_DECLARE_RESULT(SyncError)
    virtual ~SyncService() = default;

    virtual coro::task<Result<void>> sync(const PackageSectionDTO section,
                                          const RequestContext context) = 0;
    virtual coro::task<Result<void>> sync_all(const RequestContext context) = 0;
};

} // namespace bxt::Core::Application
