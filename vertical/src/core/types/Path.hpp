/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include <filesystem>

#include "infra/reflect/StringReflector.hpp"

namespace bxt {

using Path = std::filesystem::path;

} // namespace bxt

BXT_REFLECT_DECLARE_STRING_REFLECTOR(std::filesystem::path)
