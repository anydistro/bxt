################################################################################
# Set up db-cli utility
################################################################################

include("${CMAKE_SOURCE_DIR}/cmake/db-cli-deps.cmake")

add_executable(db-cli
  cli.cpp  
  validation.h
  ../daemon/core/domain/enums/PoolLocation.cpp
  ../daemon/utilities/alpmdb/Desc.cpp
  ../daemon/utilities/alpmdb/PkgInfo.cpp
  ../daemon/utilities/alpmdb/DescFormatter.cpp
  ../daemon/utilities/libarchive/Reader.cpp
)

target_link_libraries(db-cli PRIVATE db-cli-deps)

target_include_directories(
  db-cli
  PRIVATE
    ${CURL_INCLUDE_DIR}
    ${lmdbxx_SOURCE_DIR}/include
    ../daemon
)
