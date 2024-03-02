/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "JwtFilter.h"

#include <jwt-cpp/traits/nlohmann-json/defaults.h>

namespace bxt::Presentation {
using namespace drogon;

void JwtFilter::doFilter(const HttpRequestPtr &request,
                         FilterCallback &&fcb,
                         FilterChainCallback &&fccb) {
    if (request->getMethod() == HttpMethod::Options) return fccb();

    std::string token = request->getCookie("token");

    if (token.empty()) {
        Json::Value resultJson;
        resultJson["error"] =
            "Authentification header is not found or malformed";
        resultJson["status"] = "error";

        auto res = HttpResponse::newHttpJsonResponse(resultJson);
        res->setStatusCode(k401Unauthorized);

        return fcb(res);
    }

    auto decoded = jwt::decode(token);
    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256 {"secret"})
                        .with_issuer("auth0");

    std::error_code ec;

    verifier.verify(decoded, ec);

    if (ec) {
        Json::Value resultJson;
        resultJson["message"] = "Token is invalid!";
        resultJson["status"] = "error";

        auto res = HttpResponse::newHttpJsonResponse(resultJson);
        res->setStatusCode(k401Unauthorized);

        return fcb(res);
    }
    auto claims = decoded.get_payload_claims();

    for (auto &claim : claims)
        request->getAttributes()->insert("jwt_" + claim.first, claim.second);

    // If everything is right, just move to other endpoint
    return fccb();
}

} // namespace bxt::Presentation
