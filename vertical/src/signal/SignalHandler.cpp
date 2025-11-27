/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

// Signal-safe stack tracing implementation - shamelessly borrowed from cpptrace documentation
// https://raw.githubusercontent.com/jeremy-rifkin/cpptrace/refs/heads/main/docs/signal-safe-tracing.md

#include "SignalHandler.hpp"

// Disable linting for signal-safe code - we can't really do this smarter or more performant
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wmissing-braces"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"

#include <cstring>

#include <cpptrace/cpptrace.hpp>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

namespace bxt {

// This is just a utility I like, it makes the pipe API more expressive.
struct pipe_t {
    union {
        struct {
            int read_end;
            int write_end;
        };
        int data[2];
    };
};

void do_signal_safe_trace(cpptrace::frame_ptr* buffer, std::size_t count) {
    // Setup pipe and spawn child
    pipe_t input_pipe;
    pipe(input_pipe.data);
    pid_t const pid = fork();
    if (pid == -1) {
        char const* fork_failure_message = "fork() failed\n";
        write(STDERR_FILENO, fork_failure_message, strlen(fork_failure_message));
        return;
    }
    if (pid == 0) { // child
        dup2(input_pipe.read_end, STDIN_FILENO);
        close(input_pipe.read_end);
        close(input_pipe.write_end);
        execl("signal_tracer", "signal_tracer", nullptr);
        char const* exec_failure_message =
            "exec(signal_tracer) failed: Make sure the signal_tracer executable is in "
            "the current working directory and the binary's permissions are correct.\n";
        write(STDERR_FILENO, exec_failure_message, strlen(exec_failure_message));
        _exit(1);
    }
    // Resolve to safe_object_frames and write those to the pipe
    for (std::size_t i = 0; i < count; i++) {
        cpptrace::safe_object_frame frame;
        cpptrace::get_safe_object_frame(buffer[i], &frame);
        write(input_pipe.write_end, &frame, sizeof(frame));
    }
    close(input_pipe.read_end);
    close(input_pipe.write_end);
    // Wait for child
    waitpid(pid, nullptr, 0);
}

void handler(int signo, siginfo_t* info, void* context) {
    // Print basic message
    char const* message = "SIGSEGV occurred:\n";
    write(STDERR_FILENO, message, strlen(message));
    // Generate trace
    constexpr std::size_t N = 100;
    cpptrace::frame_ptr buffer[N];
    std::size_t count = cpptrace::safe_generate_raw_trace(buffer, N);
    do_signal_safe_trace(buffer, count);
    // Up to you if you want to exit or continue or whatever
    _exit(1);
}

void warmup_cpptrace() {
    // This is done for any dynamic-loading shenanigans
    cpptrace::frame_ptr buffer[10];
    std::size_t count = cpptrace::safe_generate_raw_trace(buffer, 10);
    cpptrace::safe_object_frame frame;
    if (count > 0) {
        cpptrace::get_safe_object_frame(buffer[0], &frame);
    }
}

void setup_signal_safe_tracing() {
    // Check if signal-safe tracing is supported
    if (!cpptrace::can_signal_safe_unwind()) {
        // Silently skip setup if not supported
        return;
    }

    if (!cpptrace::can_get_safe_object_frame()) {
        // Silently skip setup if object frame resolution is not supported
        return;
    }

    warmup_cpptrace();

    // Setup signal handler
    struct sigaction action {};
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = &handler;

    // Setup handlers for common fatal signals
    if (sigaction(SIGSEGV, &action, NULL) == -1) {
        // Silently ignore setup failures
    }
    if (sigaction(SIGABRT, &action, NULL) == -1) {
        // Silently ignore setup failures
    }
    if (sigaction(SIGFPE, &action, NULL) == -1) {
        // Silently ignore setup failures
    }
    if (sigaction(SIGILL, &action, NULL) == -1) {
        // Silently ignore setup failures
    }
}

} // namespace bxt

#pragma GCC diagnostic pop
#pragma clang diagnostic pop
