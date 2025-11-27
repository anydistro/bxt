/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

// Signal tracer implementation - shamelessly borrowed from cpptrace documentation
// https://raw.githubusercontent.com/jeremy-rifkin/cpptrace/refs/heads/main/docs/signal-safe-tracing.md

// Disable linting for signal-safe code
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#include <cstdio>
#include <iostream>

#include <cpptrace/cpptrace.hpp>
#include <unistd.h>

int main() {
    cpptrace::object_trace trace;
    while (true) {
        cpptrace::safe_object_frame frame;
        // fread used over read because a read() from a pipe might not read the full frame
        std::size_t res = fread(&frame, sizeof(frame), 1, stdin);
        if (res == 0) {
            break;
        } else if (res != 1) {
            std::cerr << "Something went wrong while reading from the pipe" << res << " "
                      << std::endl;
            break;
        } else {
            trace.frames.push_back(frame.resolve());
        }
    }
    trace.resolve().print();
}

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
