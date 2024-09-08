/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#include "Token.h"

#include <fmt/format.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>

namespace bxt::Presentation {

constexpr auto AccessTokenExpiration = std::chrono::minutes {15};
constexpr auto RefreshTokenExpiration = std::chrono::weeks {2};

std::expected<Token, std::string> bxt::Presentation::Token::verify_jwt(std::string const& jwt,
                                                                       std::string const& issuer,
                                                                       std::string const& secret) {
    try {
        auto const decoded = jwt::decode(jwt);
        auto const verifier =
            jwt::verify().allow_algorithm(jwt::algorithm::hs256 {secret}).with_issuer(issuer);

        verifier.verify(decoded);

        if (!decoded.has_payload_claim(Names::Storage)) {
            return std::unexpected("No token storage provided");
        }

        if (!decoded.has_payload_claim(Names::TokenKind)
            || !decoded.has_payload_claim(Names::UserName)) {
            return std::unexpected("No token kind or username provided");
        }

        auto storage = decoded.get_payload_claim(Names::Storage).as_string();
        auto kind = decoded.get_payload_claim(Names::TokenKind).as_string();
        auto username = decoded.get_payload_claim(Names::UserName).as_string();

        return Token {username,
                      kind == Names::AccessToken ? Token::Kind::Access : Token::Kind::Refresh,
                      storage == Names::CookieStorage ? Token::Storage::Cookie
                                                      : Token::Storage::Bearer,
                      decoded.get_issued_at(),
                      decoded.get_expires_at(),
                      jwt};
    } catch (std::exception const& exception) {
        return std::unexpected(
            fmt::format("Token is invalid, the error is: \"{}\"", exception.what()));
    }
}

bxt::Presentation::Token::Token(std::string name, Kind kind, Storage storage)
    : m_kind(kind)
    , m_storage(storage)
    , m_name(std::move(name))
    , m_issued_at(std::chrono::system_clock::now()) {
    using namespace std::chrono_literals;
    if (kind == Kind::Access) {
        m_expires_at = m_issued_at + AccessTokenExpiration;
    } else {
        m_expires_at = m_issued_at + RefreshTokenExpiration;
    }
}

std::string bxt::Presentation::Token::generate_jwt(std::string const& issuer,
                                                   std::string const& secret) {
    if (m_cached_jwt.has_value()) {
        try {
            auto const decoded = jwt::decode(*m_cached_jwt);
            auto const verifier =
                jwt::verify().allow_algorithm(jwt::algorithm::hs256 {secret}).with_issuer(issuer);

            verifier.verify(decoded);
            return *m_cached_jwt;
        } catch (std::exception const& exception) {
        }
    }

    m_cached_jwt = jwt::create()
                       .set_payload_claim(Names::UserName, m_name)
                       .set_payload_claim(Names::Storage, bxt::to_string(m_storage))
                       .set_payload_claim(Names::TokenKind, bxt::to_string(m_kind))
                       .set_issuer(issuer)
                       .set_type("JWS")
                       .set_issued_at(m_issued_at)
                       .set_expires_at(m_expires_at)
                       .sign(jwt::algorithm::hs256 {secret});

    return *m_cached_jwt;
}
} // namespace bxt::Presentation
