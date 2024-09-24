################################################################################
# Test Configuration
################################################################################

find_package(Catch2 REQUIRED)

################################################################################
# Test Sources
################################################################################

file(GLOB_RECURSE TEST_SOURCES "src/unit/*/**.cpp")
file(GLOB_RECURSE BXT_SOURCES "../*/**.cpp")

################################################################################
# Test Executable
################################################################################

add_executable(daemon_tests 
    ${TEST_SOURCES} ${BXT_SOURCES}
)

target_link_libraries(daemon_tests PRIVATE 
    Catch2::Catch2WithMain 
    deps 
    reflectcpp 
    Dexode::EventBus
)

target_include_directories(daemon_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../ 
    ${lmdbxx_SOURCE_DIR}/include
)

set_target_properties(daemon_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests
)

get_target_property(TESTS_RUNTIME_OUTPUT_DIRECTORY daemon_tests RUNTIME_OUTPUT_DIRECTORY)

message("Tests will be run from ${TESTS_RUNTIME_OUTPUT_DIRECTORY}")

################################################################################
# Test Data
################################################################################

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${TESTS_RUNTIME_OUTPUT_DIRECTORY})

################################################################################
# CTest Integration
################################################################################
include(Catch)

catch_discover_tests(daemon_tests WORKING_DIRECTORY ${TESTS_RUNTIME_OUTPUT_DIRECTORY})
