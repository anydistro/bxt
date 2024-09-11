################################################################################
# Swagger UI Fetching and Deployment
################################################################################

FetchContent_Declare(
    swagger-ui
    GIT_REPOSITORY https://github.com/swagger-api/swagger-ui.git
    GIT_TAG v5.15.2
    EXCLUDE_FROM_ALL
    GIT_PROGRESS TRUE
    SOURCE_SUBDIR non_existent_subdir # Prevent FetchContent_MakeAvailable() from calling add_subdirectory()
                                      # https://discourse.cmake.org/t/fetchcontent-a-directory-but-add-a-subdirectory/8603/15
)

FetchContent_MakeAvailable(swagger-ui)

add_custom_target(
    deploy_swagger
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${swagger-ui_SOURCE_DIR}/dist/
            ${CMAKE_BINARY_DIR}/bin/web/swagger/
    COMMAND ${CMAKE_COMMAND} 
            -DLIST_DIR=${CMAKE_CURRENT_LIST_DIR} 
            -DBINARY_DIR=${CMAKE_BINARY_DIR} 
            -DBXT_BASE_PATH=${BXT_BASE_URL} 
            -P "${CMAKE_CURRENT_LIST_DIR}/configure_files.cmake"
)
