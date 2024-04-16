
FetchContent_Declare(
    swagger-ui
    GIT_REPOSITORY https://github.com/swagger-api/swagger-ui.git
    GIT_TAG v5.15.2
)
FetchContent_GetProperties(swagger-ui)
if(NOT swagger-ui_POPULATED)
    FetchContent_Populate(swagger-ui)
endif(NOT swagger-ui_POPULATED)

add_custom_target(
    deploy_swagger
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${swagger-ui_SOURCE_DIR}/dist/
            ${CMAKE_BINARY_DIR}/bin/frontend/swagger/
    COMMAND ${CMAKE_COMMAND} 
            -DLIST_DIR=${CMAKE_CURRENT_LIST_DIR} 
            -DBINARY_DIR=${CMAKE_BINARY_DIR} 
            -DBXT_BASE_PATH=${BXT_BASE_URL} 
            -P "${CMAKE_CURRENT_LIST_DIR}/configure_files.cmake")
