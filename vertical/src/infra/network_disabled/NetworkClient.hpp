/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <memory>
#include <string>

#include <coro/io_scheduler.hpp>
#include <coro/task.hpp>
#include <coro/thread_pool.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#ifdef _res
    #undef _res
#endif

#include "NetworkError.hpp"
#include "core/result_types.hpp"
#include "core/types/Path.hpp"
#include "core/types/TimePoint.hpp"
#include "core/types/to_string.hpp"
#include "core/error/kinds.hpp"

namespace bxt::network {

class NetworkClient {
    using SSLClient = httplib::SSLClient;
    using Error = httplib::Error;
    using StatusCode = httplib::StatusCode;
    using Response = httplib::Response;
    using Headers = httplib::Headers;

    using Result = bxt::err::Result<Response, NetworkError>;

public:
    explicit NetworkClient(std::shared_ptr<coro::io_scheduler> io_scheduler)
        : m_io_scheduler(std::move(io_scheduler)) {
    }

    Task<std::unique_ptr<SSLClient>> get_client(std::string const& url);

    Task<Result> download_file(std::string const& url,
                               std::string const& path,
                               std::string const& output_file = "");

    inline Task<Result> get(std::string const& url, std::string const& path) {
        return download_file(url, path);
    }

    inline Task<Result> download_to_file(std::string const& url,
                                         std::string const& path,
                                         std::string const& output_file) {
        return download_file(url, path, output_file);
    }

private:
    std::shared_ptr<coro::io_scheduler> m_io_scheduler;
};

} // namespace bxt::network
