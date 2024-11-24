################################################################################
# Dependencies for db-cli
#
# See also deps.cmake and bundled-deps.cmake
################################################################################

add_library(db-cli-deps INTERFACE)

find_package(OpenSSL REQUIRED)
target_link_libraries(db-cli-deps INTERFACE openssl::openssl)

find_package(fmt REQUIRED)
target_link_libraries(db-cli-deps INTERFACE fmt::fmt)

find_package(CLI11 REQUIRED)
target_link_libraries(db-cli-deps INTERFACE CLI11::CLI11)

find_package(lmdb REQUIRED)
target_link_libraries(db-cli-deps INTERFACE lmdb::lmdb)

find_package(cereal REQUIRED)
target_link_libraries(db-cli-deps INTERFACE cereal::cereal)

find_package(libcoro REQUIRED)
target_link_libraries(db-cli-deps INTERFACE libcoro::libcoro)

find_package(Boost REQUIRED)
target_link_libraries(db-cli-deps INTERFACE boost::boost)

find_package(frozen REQUIRED)
target_link_libraries(db-cli-deps INTERFACE frozen::frozen)

find_package(LibArchive REQUIRED)
target_link_libraries(db-cli-deps INTERFACE LibArchive::LibArchive)

find_package(phmap REQUIRED)
target_link_libraries(db-cli-deps INTERFACE phmap)


################################################################################
# Bundled Dependencies for db-cli
#
################################################################################

include(FetchContent)

FetchContent_Declare(
    lmdbxx
    GIT_REPOSITORY https://github.com/hoytech/lmdbxx
    GIT_TAG        b12d1b12f4c9793aef8bed03d07ecbf789e810b4
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(lmdbxx)
