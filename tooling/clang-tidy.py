#!/usr/bin/env python

# SPDX-FileCopyrightText: 2024 Roman Gilg <subdiff@gmail.com>
# SPDX-License-Identifier: MIT

import argparse, os, subprocess, sys, tempfile
from shutil import which

script_dir = os.path.dirname(os.path.realpath(__file__))
source_dir = os.path.dirname(os.path.dirname(script_dir))


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Run clang-tidy script with additional options"
    )
    parser.add_argument(
        "-p",
        dest="build_path",
        required=True,
        help="path used to read a compile command database",
    )
    parser.add_argument(
        "-j",
        type=int,
        default=0,
        help="number of tidy instances to be run in parallel",
    )
    parser.add_argument(
        "-clang-version",
        dest="clang_version",
        type=int,
        default=-1,
        help="clang-version to use (defaults to development version)",
    )
    parser.add_argument(
        "-clang-tidy-binary",
        dest="clang_tidy_path",
        default="",
        help="path to clant-tidy binary",
    )
    parser.add_argument(
        "-header-filter",
        dest="header_filter",
        default=".*",
        help="regular expression matching the names of the headers to output diagnostics from",
    )
    return parser.parse_args()


args = parse_arguments()
os.chdir(source_dir)


def get_binary_args():
    add_args = ["-clang-tidy-binary"]

    if args.clang_tidy_path:
        return add_args + [args.clang_tidy_path]

    if args.clang_version > 0:
        versioned_path = which("clang-tidy-" + str(args.clang_version))
        if versioned_path is not None:
            return add_args + [versioned_path]
    return []


# Additional arguments for run-clang-tidy.py
additional_args = [
    "-use-color",
    "-j",
    str(args.j),
    "-p=" + args.build_path,
    "-header-filter=" + args.header_filter,
]
additional_args += get_binary_args()

if args.clang_version > 0:
    run_script_url = (
        "https://raw.githubusercontent.com/llvm/llvm-project/release/"
        + str(args.clang_version)
        + ".x/clang-tools-extra/clang-tidy/tool/run-clang-tidy.py"
    )
else:
    run_script_url = "https://raw.githubusercontent.com/llvm/llvm-project/main/clang-tools-extra/clang-tidy/tool/run-clang-tidy.py"

# Download the run-clang-tidy.py script and save it as a temporary file
with tempfile.NamedTemporaryFile(mode="w") as temp_file:
    curl_process = subprocess.Popen(["curl", "-s", run_script_url], stdout=temp_file)
    curl_process.wait()
    run_clang_tidy_script_path = temp_file.name

    # Execute the modified script using Python with additional arguments
    python_process = subprocess.Popen(
        [sys.executable, run_clang_tidy_script_path] + additional_args + [source_dir],
        stdin=subprocess.PIPE,
    )
    python_process.communicate()
    exit(python_process.returncode)
