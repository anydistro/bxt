/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <yaml-cpp/node/node.h>
namespace bxt::Utilities::RepoSchema {
struct Extension {
    virtual void parse(const YAML::Node& root_node) = 0;
};
} // namespace bxt::Utilities::RepoSchema
