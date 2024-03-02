/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include "core/application/services/AuthService.h"

#include <drogon/HttpFilter.h>

namespace bxt::Presentation {

class JwtFilter : public drogon::HttpFilter<JwtFilter, false> {
public:
    JwtFilter(Core::Application::AuthService &service) : m_service(service) {}

    virtual void doFilter(const drogon::HttpRequestPtr &req,
                          drogon::FilterCallback &&fcb,
                          drogon::FilterChainCallback &&fccb) override;

private:
    Core::Application::AuthService &m_service;
};

} // namespace bxt::Presentation
