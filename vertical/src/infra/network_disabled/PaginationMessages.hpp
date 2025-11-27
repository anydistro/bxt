/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

namespace bxt::network {

constexpr auto PAGE_QUERY_PARAM = "page";
constexpr auto LIMIT_QUERY_PARAM = "limit";

struct PaginationRequest {
    int page = 1;
    int limit = 10;
};

struct PaginationResponse {
    int page = 1;
    int limit = 10;
    int total = 0;
};

} // namespace bxt::network
