
################################################################################
# Project Configuration
################################################################################
cmake_minimum_required(VERSION 3.16)
project(bxtnext LANGUAGES CXX)

################################################################################
# Options
################################################################################
set(BXT_LMDB_DATA_FORMAT "msgpack" CACHE STRING "Data format for LMDB storage (all supported by reflect-cpp)")


################################################################################
# Dependencies and Sources
################################################################################
file(GLOB_RECURSE FEATURE_SOURCES CONFIGURE_DEPENDS "features/*.cpp")
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "core/*.cpp")
file(GLOB_RECURSE INFRA_SOURCES CONFIGURE_DEPENDS "infra/*.cpp")
file(GLOB_RECURSE UTILS_SOURCES CONFIGURE_DEPENDS "utils/*.cpp")
file(GLOB_RECURSE SIGNAL_SOURCES CONFIGURE_DEPENDS "signal/*.cpp")

list(FILTER FEATURE_SOURCES EXCLUDE REGEX "_test\\.cpp$")
list(FILTER CORE_SOURCES EXCLUDE REGEX "_test\\.cpp$")
list(FILTER INFRA_SOURCES EXCLUDE REGEX "_test\\.cpp$")
list(FILTER UTILS_SOURCES EXCLUDE REGEX "_test\\.cpp$")
list(FILTER SIGNAL_SOURCES EXCLUDE REGEX "_test\\.cpp$")

list(FILTER FEATURE_SOURCES EXCLUDE REGEX "_disabled/")
list(FILTER CORE_SOURCES EXCLUDE REGEX "_disabled/")
list(FILTER INFRA_SOURCES EXCLUDE REGEX "_disabled/")
list(FILTER UTILS_SOURCES EXCLUDE REGEX "_disabled/")
list(FILTER SIGNAL_SOURCES EXCLUDE REGEX "_disabled/")

set(ALL_SOURCES ${FEATURE_SOURCES} ${CORE_SOURCES} ${INFRA_SOURCES} ${UTILS_SOURCES} ${SIGNAL_SOURCES})



if(BUILD_TESTING)

    ################################################################################
    # Tests configuration
    ################################################################################

    find_package(Catch2 REQUIRED)

    ################################################################################
    # Test Sources
    ################################################################################

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*_test.cpp")

    list(FILTER TEST_SOURCES EXCLUDE REGEX "_disabled/")

    add_executable(${PROJECT_NAME}_tests 
        ${TEST_SOURCES} ${ALL_SOURCES}
    )

    target_include_directories(${PROJECT_NAME}_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/daemon
        ${CMAKE_SOURCE_DIR}/cpperr/include
    )

    target_compile_definitions(${PROJECT_NAME}_tests PRIVATE
        BXT_LMDB_DATA_FORMAT=${BXT_LMDB_DATA_FORMAT}
    )

    target_compile_options(${PROJECT_NAME}_tests PRIVATE
        -Wall
        -Wextra
        -Werror
    )

    set_target_properties(${PROJECT_NAME}_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests"
    )

    target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        Catch2::Catch2WithMain
        deps
        reflectcpp
        Dexode::EventBus
    )

    get_target_property(TESTS_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_NAME}_tests RUNTIME_OUTPUT_DIRECTORY)

    message("Tests will be launched from ${TESTS_RUNTIME_OUTPUT_DIRECTORY}")

    ################################################################################
    # CTest Integration
    ################################################################################
    include(Catch)

    catch_discover_tests(${PROJECT_NAME}_tests WORKING_DIRECTORY ${TESTS_RUNTIME_OUTPUT_DIRECTORY})
endif()

################################################################################
# Executable Configuration
################################################################################
add_executable(${PROJECT_NAME} 
    ${ALL_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/daemon
    ${CMAKE_SOURCE_DIR}/cpperr/include
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    REFLECTCPP_USE_STD_EXPECTED=1
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    deps
    Dexode::EventBus
    reflectcpp
    spdlog::spdlog_header_only
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_tests)
