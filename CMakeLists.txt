################################################################################
# CMake Configuration: Set up project and compiler settings
################################################################################
cmake_minimum_required(VERSION 3.23)
project(bxt C CXX)
include(CTest)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(FETCHCONTENT_QUIET FALSE)

option(BXT_EXPERIMENTAL_COPY_MOVE "Enable experimental copy/move operations" OFF)

################################################################################
# Dependencies: Fetch and configure external libraries not available in Conan
################################################################################
include("${CMAKE_SOURCE_DIR}/cmake/bundled-deps.cmake")

################################################################################
# Dependencies: Configure dependencies available via Conan
################################################################################
include("${CMAKE_SOURCE_DIR}/cmake/deps.cmake")

################################################################################
# Static Analysis: Enable Clang-Tidy
################################################################################
option(ENABLE_CLANG_TIDY "Enable Clang-Tidy checks" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-18)

    if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=${CMAKE_SOURCE_DIR}/daemon/.*")
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    else()
    message(SEND_ERROR "clang-tidy not found.")
    endif()
endif()

################################################################################
# Project Structure: Add subdirectories for different components
################################################################################
add_subdirectory(daemon)
add_subdirectory(web)
add_subdirectory(db-cli)
