cmake_minimum_required(VERSION 3.16)
project(bxtd LANGUAGES CXX)

add_subdirectory(swagger)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(
    ./
    ${CURL_INCLUDE_DIR}
    ${lmdbxx_SOURCE_DIR}/include
)

add_compile_definitions(${PROJECT_NAME} PRIVATE
    BOOST_ASIO_HAS_CO_AWAIT=1
    BOOST_ASIO_HAS_STD_COROUTINE=1
    BOOST_LOG_DYN_LINK=1
    TOML_EXCEPTIONS=0)

target_compile_options(${PROJECT_NAME} PUBLIC -fcoroutines)

target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/bin)
target_link_libraries(${PROJECT_NAME}
    ${CONAN_LIBS}
    Dexode::EventBus
    reflectcpp
)

add_dependencies(${PROJECT_NAME} deploy_swagger)
