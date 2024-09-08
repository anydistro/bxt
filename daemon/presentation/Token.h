/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "presentation/Names.h"
#include "utilities/to_string.h"

#include <chrono>
#include <expected>
#include <optional>
#include <string>
#include <utility>

namespace bxt::Presentation {

class Token {
public:
    using time_point = std::chrono::system_clock::time_point;
    enum Kind { Access, Refresh };
    enum Storage { Cookie, Bearer };
    Token(std::string name, Kind kind, Storage storage);

    std::string generate_jwt(std::string const& issuer, std::string const& secret);

    static std::expected<Token, std::string>
        verify_jwt(std::string const& jwt, std::string const& issuer, std::string const& secret);

    Kind kind() const {
        return m_kind;
    }

    Storage storage() const {
        return m_storage;
    }

    std::string name() const {
        return m_name;
    }

    time_point issued_at() const {
        return m_issued_at;
    }

    time_point expires_at() const {
        return m_expires_at;
    }

private:
    Token(std::string name,
          Kind kind,
          Storage storage,
          time_point issued_at,
          time_point expires_at,
          std::optional<std::string> cached_jwt = std::nullopt)
        : m_kind(kind)
        , m_storage(storage)
        , m_name(std::move(name))
        , m_issued_at(issued_at)
        , m_expires_at(expires_at)
        , m_cached_jwt(std::move(cached_jwt)) {
    }
    Kind m_kind = Kind::Access;
    Storage m_storage = Storage::Cookie;
    std::string m_name;
    time_point m_issued_at;
    time_point m_expires_at;
    std::optional<std::string> m_cached_jwt = std::nullopt;
};
} // namespace bxt::Presentation

template<> inline std::string bxt::to_string(Presentation::Token::Storage const& storage) {
    switch (storage) {
    case Presentation::Token::Storage::Cookie:
        return Presentation::Names::CookieStorage;
    case Presentation::Token::Storage::Bearer:
        return Presentation::Names::BearerStorage;
    default:
        return "Unknown";
    }
}

template<> inline std::string bxt::to_string(Presentation::Token::Kind const& kind) {
    switch (kind) {
    case Presentation::Token::Kind::Access:
        return Presentation::Names::AccessToken;
    case Presentation::Token::Kind::Refresh:
        return Presentation::Names::RefreshToken;
    default:
        return "Unknown";
    }
}
