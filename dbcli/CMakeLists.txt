add_executable(dbcli dbcli.cpp ../daemon/core/domain/enums/PoolLocation.cpp)

target_link_libraries(dbcli PRIVATE ${CONAN_LIBS})

target_include_directories(
  dbcli PRIVATE ${CURL_INCLUDE_DIR} ${lmdbxx_SOURCE_DIR}/include ../daemon/)
