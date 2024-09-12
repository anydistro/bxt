################################################################################
# Bundled Dependencies
#
# Ideally there should be none but there are some dependencies that are
# not available in the package manager.
################################################################################

include(FetchContent)

FetchContent_Declare(
    lmdbxx
    GIT_REPOSITORY https://github.com/hoytech/lmdbxx
    GIT_TAG        b12d1b12f4c9793aef8bed03d07ecbf789e810b4
    GIT_PROGRESS TRUE
)

# Related PR: https://github.com/gelldur/EventBus/pull/55
set(eventbus_patch git apply ${CMAKE_CURRENT_LIST_DIR}/patches/eventbus.patch)

FetchContent_Declare(
    event-bus
    GIT_REPOSITORY https://github.com/gelldur/EventBus.git
    GIT_TAG        v3.1.2
    EXCLUDE_FROM_ALL
    GIT_PROGRESS TRUE
    PATCH_COMMAND ${eventbus_patch}
    UPDATE_DISCONNECTED 1
    SOURCE_SUBDIR non_existent_subdir # Prevent FetchContent_MakeAvailable() from calling add_subdirectory()
                                      # https://discourse.cmake.org/t/fetchcontent-a-directory-but-add-a-subdirectory/8603/15
)

FetchContent_Declare(
    reflect-cpp
    GIT_REPOSITORY https://github.com/getml/reflect-cpp.git
    GIT_TAG v0.14.1
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(reflect-cpp)

FetchContent_MakeAvailable(lmdbxx event-bus)

add_subdirectory(${event-bus_SOURCE_DIR}/lib ${event-bus_BINARY_DIR} SYSTEM)
