# Simple library loading, handy for header only libraries
function(LoadDependency uri relpath)
    set(outmsg "Check ${relpath}...")
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${relpath})
        message(STATUS "${outmsg} Already exists")
    else()
        message(STATUS "${outmsg} Download ${relpath}")

        set(result_list "")
        set(code "")
        file(DOWNLOAD ${uri} ${CMAKE_CURRENT_SOURCE_DIR}/${relpath} STATUS result_list)
        list(GET result_list 0 code)

        if (${code})
            set(error "")
            list(GET result_list 1 error)
            message(SEND_ERROR "Error downloading ${relpath}: ${error}")
        else()
            message(STATUS "Done")
        endif()

    endif()
endfunction()

# Assert that library exist in system and set ${Var}_LIBRARY variable
function(RequireLibrary Var LibName)
    find_library(${Var}_LIBRARY ${LibName})
    if (NOT ${Var}_LIBRARY)
        message(FATAL_ERROR "Cannot find library ${LibName}")
    else()
        message(STATUS "${Var} library: ${${Var}_LIBRARY}")
    endif()
endfunction()

# Symlink installer, handy for resources handling
macro(InstallSymlink filepath sympath)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${filepath} ${sympath})")
    install(CODE "message(\"-- Created symlink: ${sympath} -> ${filepath}\")")
endmacro(InstallSymlink)