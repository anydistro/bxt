################################################################################
# Dependencies
#
# See also conanfile.py in a project root.
################################################################################

add_library(deps INTERFACE)

find_package(OpenSSL REQUIRED)
target_link_libraries(deps INTERFACE openssl::openssl)

find_package(Boost REQUIRED)
target_link_libraries(deps INTERFACE boost::boost)

find_package(date REQUIRED)
target_link_libraries(deps INTERFACE date::date)

find_package(fmt REQUIRED)
target_link_libraries(deps INTERFACE fmt::fmt)

find_package(frozen REQUIRED)
target_link_libraries(deps INTERFACE frozen::frozen)

find_package(yaml-cpp REQUIRED)
target_link_libraries(deps INTERFACE yaml-cpp::yaml-cpp)

find_package(tomlplusplus REQUIRED)
target_link_libraries(deps INTERFACE tomlplusplus::tomlplusplus)

find_package(jwt-cpp REQUIRED)
target_link_libraries(deps INTERFACE jwt-cpp::jwt-cpp)

find_package(httplib REQUIRED)
target_link_libraries(deps INTERFACE httplib::httplib)

find_package(phmap REQUIRED)
target_link_libraries(deps INTERFACE phmap)

find_package(LibArchive REQUIRED)
target_link_libraries(deps INTERFACE LibArchive::LibArchive)

find_package(Drogon REQUIRED)
target_link_libraries(deps INTERFACE Drogon::Drogon)

find_package(kangaru REQUIRED)
target_link_libraries(deps INTERFACE kangaru)

find_package(lmdb REQUIRED)
target_link_libraries(deps INTERFACE lmdb::lmdb)

find_package(nlohmann_json REQUIRED)
target_link_libraries(deps INTERFACE nlohmann_json::nlohmann_json)

find_package(scn REQUIRED)
target_link_libraries(deps INTERFACE scn::scn)

find_package(cereal REQUIRED)
target_link_libraries(deps INTERFACE cereal::cereal)

find_package(libcoro REQUIRED)
target_link_libraries(deps INTERFACE libcoro::libcoro)

find_package(scope-lite REQUIRED)
target_link_libraries(deps INTERFACE nonstd::scope-lite)
