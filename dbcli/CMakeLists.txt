add_executable(
  dbcli
  dbcli.cpp ../daemon/core/domain/enums/PoolLocation.cpp
  ../daemon/utilities/alpmdb/Desc.cpp ../daemon/utilities/alpmdb/PkgInfo.cpp
  ../daemon/utilities/libarchive/Reader.cpp)

target_link_libraries(dbcli PRIVATE ${CONAN_LIBS})

target_include_directories(
  dbcli PRIVATE ${CURL_INCLUDE_DIR} ${lmdbxx_SOURCE_DIR}/include ../daemon/)
