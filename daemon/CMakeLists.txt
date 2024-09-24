################################################################################
# Project Configuration
################################################################################
cmake_minimum_required(VERSION 3.16)
project(bxtd LANGUAGES CXX)

################################################################################
# Dependencies and Sources
################################################################################
add_subdirectory(swagger)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "tests/.*")

################################################################################
# Executable Configuration
################################################################################
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CURL_INCLUDE_DIR}
    ${lmdbxx_SOURCE_DIR}/include
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    BOOST_ASIO_HAS_CO_AWAIT=1
    BOOST_ASIO_HAS_STD_COROUTINE=1
    BOOST_LOG_DYN_LINK=1
    TOML_EXCEPTIONS=0
    BXT_EXPERIMENTAL_COPY_MOVE=$<BOOL:${BXT_EXPERIMENTAL_COPY_MOVE}>
)

target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    deps
    Dexode::EventBus
    reflectcpp
)

add_dependencies(${PROJECT_NAME} deploy_swagger daemon_tests)
