# Percpptency - Persistence for C++ Entities
# SPDX-FileCopyrightText: 2025 Artem Grinev <agrinev@manjaro.org>
# SPDX-License-Identifier: AGPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)
project(percpptency VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent for dependencies
include(FetchContent)

# reflect-cpp - automatic serialization
FetchContent_Declare(
    reflect-cpp
    GIT_REPOSITORY https://github.com/getml/reflect-cpp.git
    GIT_TAG v0.15.0
    GIT_SHALLOW TRUE
)

# sqlgen - type-safe SQL query builder
FetchContent_Declare(
    sqlgen
    GIT_REPOSITORY https://github.com/tobias-loew/sqlgen.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

# eventpp - event dispatcher
FetchContent_Declare(
    eventpp
    GIT_REPOSITORY https://github.com/wqking/eventpp.git
    GIT_TAG v0.1.3
    GIT_SHALLOW TRUE
)

# frozen - constexpr containers
FetchContent_Declare(
    frozen
    GIT_REPOSITORY https://github.com/serge-sans-paille/frozen.git
    GIT_TAG 1.1.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(reflect-cpp sqlgen eventpp frozen)

# Core library - no external dependencies except std
add_library(percpptency_core INTERFACE)
target_include_directories(percpptency_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_sources(percpptency_core INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        core/AggregateRoot.hpp
        core/DomainEvent.hpp
        core/Repository.hpp
        core/UnitOfWork.hpp
)

# DomainEvent implementation (needs compilation)
add_library(percpptency_core_impl
    core/DomainEvent.cpp
)
target_link_libraries(percpptency_core_impl PUBLIC percpptency_core)

# SQLite/sqlgen adapter - requires reflect-cpp, sqlgen, cpperr, coro
add_library(percpptency_sqlgen INTERFACE)
target_include_directories(percpptency_sqlgen INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_sources(percpptency_sqlgen INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        adapters/sqlgen/SqlgenError.hpp
        adapters/sqlgen/SqlgenRepository.hpp
        adapters/sqlgen/SqlgenUnitOfWork.hpp
        adapters/sqlgen/SqlgenUnitOfWorkFactory.hpp
)

target_link_libraries(percpptency_sqlgen INTERFACE
    percpptency_core
    rfl
    sqlgen
    eventpp::eventpp
    frozen::frozen
    # Parent project must provide:
    # - cpperr (from bxt workspace)
    # - coro (from drogon)
)

# Alias for use in parent projects
add_library(percpptency::core ALIAS percpptency_core)
add_library(percpptency::core_impl ALIAS percpptency_core_impl)
add_library(percpptency::sqlgen ALIAS percpptency_sqlgen)

# Installation
install(TARGETS percpptency_core percpptency_core_impl percpptency_sqlgen
    EXPORT percpptencyTargets
    FILE_SET HEADERS
)

install(EXPORT percpptencyTargets
    FILE percpptencyTargets.cmake
    NAMESPACE percpptency::
    DESTINATION lib/cmake/percpptency
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/percpptencyConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/percpptencyConfigVersion.cmake"
    DESTINATION lib/cmake/percpptency
)

# Tests
option(PERCPPTENCY_BUILD_TESTS "Build tests for percpptency" OFF)

if(PERCPPTENCY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
