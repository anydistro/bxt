/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <string>

namespace bxt::Core::Application {
struct Notification {
public:
    std::string title;
    std::string content;

    bool valid() const { return !title.empty(); }
};

} // namespace bxt::Core::Application
