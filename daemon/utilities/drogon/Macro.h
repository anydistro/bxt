/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#define BXT_ADD_METHOD_TO(...) ADD_METHOD_TO(__VA_ARGS__, drogon::Options)

#define BXT_JWT_ADD_METHOD_TO(...) \
    ADD_METHOD_TO(__VA_ARGS__, drogon::Options, "bxt::Presentation::JwtFilter")
