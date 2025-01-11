#pragma once

// Based on https://github.com/jeremy-rifkin/cpptrace/blob/main/docs/signal-safe-tracing.md

#include <cpptrace/cpptrace.hpp>
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

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
    cpptrace::get_safe_object_frame(buffer[0], &frame);
}

void register_signal_handler() {
    warmup_cpptrace();
    struct sigaction action {};
    action.sa_flags = 0;
    action.sa_sigaction = &handler;
    action.sa_mask = {};

    sigaction(SIGSEGV, &action, nullptr);
}
