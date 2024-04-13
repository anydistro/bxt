/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once
#include <expected>

#define BXT_DECLARE_RESULT(error)        \
    template<typename TBxtExpectedValue> \
    using Result = std::expected<TBxtExpectedValue, error>;
