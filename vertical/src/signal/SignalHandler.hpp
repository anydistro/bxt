/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

namespace bxt {

/**
 * @brief Initialize signal-safe stack tracing for crash handling
 *
 * This sets up signal handlers for SIGSEGV and other fatal signals
 * to produce stack traces using cpptrace in a signal-safe manner.
 *
 * Implementation shamelessly borrowed from cpptrace documentation:
 * https://raw.githubusercontent.com/jeremy-rifkin/cpptrace/refs/heads/main/docs/signal-safe-tracing.md
 */
void setup_signal_safe_tracing();

} // namespace bxt
