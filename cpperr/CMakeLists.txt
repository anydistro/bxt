cmake_minimum_required(VERSION 3.20)
project(cpperr VERSION 1.0.0 LANGUAGES CXX)

# Header-only library
add_library(cpperr INTERFACE)
add_library(cpperr::cpperr ALIAS cpperr)

target_include_directories(cpperr INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(cpperr INTERFACE cxx_std_20)

# Installation
include(GNUInstallDirs)

install(TARGETS cpperr
    EXPORT cpperrTargets
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT cpperrTargets
    FILE cpperrTargets.cmake
    NAMESPACE cpperr::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpperr
)

# Package config
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpperrConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cpperrConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpperr
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cpperrConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cpperrConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cpperrConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cpperr
)
