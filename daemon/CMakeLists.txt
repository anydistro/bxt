cmake_minimum_required(VERSION 3.16)
project(bxtd LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -fPIC -fopenmp -DBOOST -fcoroutines")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN'")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(
    ./
    ${CURL_INCLUDE_DIR}
    ${lmdbxx_SOURCE_DIR}/include
)

add_compile_definitions(${PROJECT_NAME} PUBLIC
    BOOST_ASIO_HAS_CO_AWAIT=1
    BOOST_ASIO_HAS_STD_COROUTINE=1
    BOOST_LOG_DYN_LINK=1)

target_compile_options(${PROJECT_NAME} PUBLIC -fcoroutines)

target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/bin)
target_link_libraries(${PROJECT_NAME}
    ${CONAN_LIBS}
    Dexode::EventBus
)

target_compile_definitions(${PROJECT_NAME} PRIVATE TOML_EXCEPTIONS=0)
