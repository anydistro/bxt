# Percpptency Tests
# SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
# SPDX-License-Identifier: AGPL-3.0-or-later

find_package(Catch2 3 REQUIRED)

# Test executable with all tests
add_executable(percpptency_tests
    DomainEvent_test.cpp
    AggregateRoot_test.cpp
    AggregateReflector_test.cpp
    UnitOfWork_test.cpp
    Repository_test.cpp
)

target_link_libraries(percpptency_tests PRIVATE
    percpptency::core
    percpptency::core_impl
    rfl
    Catch2::Catch2WithMain
)

target_compile_options(percpptency_tests PRIVATE
    -stdlib=libc++
)

target_link_options(percpptency_tests PRIVATE
    -stdlib=libc++
)

include(CTest)
include(Catch)
catch_discover_tests(percpptency_tests)
