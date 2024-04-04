set(_prev "${YARN_EXECUTABLE}")
find_program(YARN_EXECUTABLE yarn DOC "Path to Yarn, the better NPM")

if(YARN_EXECUTABLE)
    if(NOT _prev)
        message(STATUS "Found Yarn executable: ${YARN_EXECUTABLE}")
    endif()
    set(Yarn_FOUND TRUE CACHE INTERNAL "")
else()
    set(Yarn_FOUND FALSE CACHE INTERNAL "")
    if(Yarn_FIND_REQUIRED)
        message(FATAL_ERROR "Failed to find a Yarn executable")
    endif()
endif()

function(yarn_run_command cmd working_dir)
  message(STATUS "Running command: ${YARN_EXECUTABLE} ${cmd} at ${working_dir}")
  execute_process(
    COMMAND ${YARN_EXECUTABLE} ${cmd}
    RESULT_VARIABLE result
    ERROR_VARIABLE error
    WORKING_DIRECTORY ${working_dir}
  )
  if (result)
    message(FATAL_ERROR "Failed to run command: ${result}")
  endif()
endfunction()

function(yarn_add_cra_project package_name working_dir)
  yarn_run_command(install ${working_dir})

  add_custom_target("${package_name}_build" ALL
    COMMAND ${CMAKE_COMMAND} -E env BUILD_PATH='${CMAKE_CURRENT_BINARY_DIR}' ${YARN_EXECUTABLE} build
    WORKING_DIRECTORY ${working_dir}
    COMMENT "Building ${package_name}"
    USES_TERMINAL
  )

  add_custom_command(
        TARGET "${package_name}_build" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_BINARY_DIR}/bin/frontend)

  add_custom_target("${package_name}_install"
    COMMAND ${CMAKE_COMMAND} -E env BUILD_PATH='${CMAKE_CURRENT_BINARY_DIR}' ${YARN_EXECUTABLE} install
    WORKING_DIRECTORY ${working_dir}
    COMMENT "Installing packages for ${package_name}"
    USES_TERMINAL
  )
endfunction()
