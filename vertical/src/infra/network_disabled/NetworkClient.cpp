/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "NetworkClient.hpp"

#include <fstream>

#include "core/error/Utils.hpp"

namespace bxt::network {

Task<std::unique_ptr<NetworkClient::SSLClient>> NetworkClient::get_client(std::string const& url) {
    using namespace std::chrono_literals;
    constexpr static auto timeout = 5s;

    co_await m_io_scheduler->schedule();

    auto client_ptr = std::make_unique<SSLClient>(url);
    client_ptr->set_follow_location(true);
    client_ptr->enable_server_certificate_verification(true);
    client_ptr->set_connection_timeout(timeout);

    co_return client_ptr;
}
Task<NetworkClient::Result> NetworkClient::download_file(std::string const& url,
                                                         std::string const& path,
                                                         std::string const& output_file) {
    using namespace std::chrono_literals;
    using namespace bxt::err;
    constexpr int retry_max = 5;
    constexpr auto delay = 50ms;

    int current_retry = 0;
    httplib::Result response;

    while (current_retry < retry_max) {
        auto client = co_await get_client(url);
        if (!client) {
            co_return make_error<NetworkError>(make_args(httplib::Error::Connection, url),
                                               "Failed to create network client");
        }

        if (!output_file.empty()) {
            std::ofstream stream(output_file, std::ios::binary);
            if (!stream.is_open()) {
                co_return make_error<NetworkError>(make_args(Error::Unknown, url));
            }

            response = client->Get(path, [&](char const* data, size_t data_length) {
                stream.write(data, static_cast<std::streamsize>(data_length));
                return stream.good();
            });

            stream.close();

            if (!stream) {
                co_return make_error<NetworkError>(make_args(Error::Unknown, url));
            }
        } else {
            response = client->Get(path, Headers());
        }

        if (response && response.error() == Error::Success
            && response->status == StatusCode::OK_200) {
            co_return *response;
        }

        co_await m_io_scheduler->yield_for(delay);
        ++current_retry;
    }

    if (!response) {
        co_return make_error<NetworkError>(make_args(Error::Unknown, url));
    }

    co_return make_error<NetworkError>(make_args(response.error(), url));
}
} // namespace bxt::network
