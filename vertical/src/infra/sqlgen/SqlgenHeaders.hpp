/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

// Save and undefine the _res macro to avoid conflicts with sqlgen
#ifdef _res
    #define BXT_SAVED_RES_MACRO _res
    #undef _res
#endif

// Include sqlgen headers
#include <sqlgen.hpp>
#include <sqlgen/delete_from.hpp>
#include <sqlgen/sqlite.hpp>
#include <sqlgen/where.hpp>

// Restore the _res macro if it was defined
#ifdef BXT_SAVED_RES_MACRO
    #define _res BXT_SAVED_RES_MACRO
    #undef BXT_SAVED_RES_MACRO
#endif
