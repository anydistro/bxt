/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "JwtFilter.h"

#include "jwt-cpp/jwt.h"
#include "presentation/Names.h"
#include "utilities/drogon/Helpers.h"

#include <expected>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>

namespace bxt::Presentation {
using namespace drogon;

std::expected<jwt::decoded_jwt<jwt::traits::nlohmann_json>, std::exception>
    decode_jwt(std::string const& token) {
    try {
        return jwt::decode(token);
    } catch (std::exception& e) {
        return std::unexpected(e);
    }
}

void JwtFilter::doFilter(HttpRequestPtr const& request,
                         FilterCallback&& fcb,
                         FilterChainCallback&& fccb) {
    if (request->getMethod() == HttpMethod::Options) {
        return fccb();
    }

    auto access_token =
        drogon_helpers::get_access_token(request, m_options.issuer, m_options.secret);
    if (!access_token.has_value()) {
        return fcb(drogon_helpers::make_error_response(access_token.error(), k401Unauthorized));
    }

    request->getAttributes()->insert(fmt::format("jwt_{}", Names::UserName), access_token->name());

    return fccb();
}

} // namespace bxt::Presentation
