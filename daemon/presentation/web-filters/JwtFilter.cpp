/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "JwtFilter.h"

#include "utilities/drogon/Helpers.h"

#include <expected>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>

namespace bxt::Presentation {
using namespace drogon;

std::expected<jwt::decoded_jwt<jwt::traits::nlohmann_json>, std::exception>
    decode_jwt(const std::string &token) {
    try {
        return jwt::decode(token);
    } catch (std::exception &e) { return std::unexpected(e); }
}

void JwtFilter::doFilter(const HttpRequestPtr &request,
                         FilterCallback &&fcb,
                         FilterChainCallback &&fccb) {
    if (request->getMethod() == HttpMethod::Options) return fccb();

    std::string token = request->getCookie("token");
    std::string provided_storage = "cookie";

    if (token.empty()) {
        auto auth_header = request->getHeader("Authorization");
        if (!auth_header.empty() && auth_header.find("Bearer ") == 0) {
            token = auth_header.substr(7);
            provided_storage = "bearer";
        }
    }

    if (token.empty()) {
        return fcb(drogon_helpers::make_error_response(
            "Authentification header is not found", k401Unauthorized));
    }

    auto verifier =
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256 {m_options.secret})
            .with_issuer("auth0");

    auto decoded = decode_jwt(token);

    if (!decoded.has_value()) {
        return fcb(drogon_helpers::make_error_response(
            fmt::format("Error while decoding the token: {}",
                        decoded.error().what()),
            k401Unauthorized));
    }

    std::error_code ec;

    verifier.verify(*decoded, ec);

    if (ec) {
        return fcb(drogon_helpers::make_error_response(
            "Authentification token is invalid", k401Unauthorized));
    }

    if (!decoded->has_payload_claim("storage")) {
        return fcb(drogon_helpers::make_error_response(
            "No token storage provided", k401Unauthorized));
    }

    auto storage = decoded->get_payload_claim("storage").as_string();
    if (storage != provided_storage) {
        return fcb(drogon_helpers::make_error_response(
            fmt::format(
                R"(Token storage is invalid, expected: "{}", got: "{}")",
                provided_storage, storage),
            k401Unauthorized));
    }

    for (auto &claim : decoded->get_payload_claims())
        request->getAttributes()->insert("jwt_" + claim.first,
                                         claim.second.as_string());

    // If everything is right, just move to other endpoint
    return fccb();
}

} // namespace bxt::Presentation
