# Resource embedding utilities

# Embed shader files as C++ raw string literals
# Usage: embed_shaders(file1.glsl file2.glsl ...)
# Creates: generated/resources/shaders/file1.glsl.h, generated/resources/shaders/file2.glsl.h, etc.
function(embed_shaders)
    foreach(SHADER_FILE ${ARGN})
        get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
        get_filename_component(SHADER_NAME_WE ${SHADER_FILE} NAME_WE)
        get_filename_component(SHADER_EXT ${SHADER_FILE} EXT)
        string(SUBSTRING ${SHADER_EXT} 1 -1 SHADER_EXT_CLEAN)

        # Create predictable output path: resources/shaders/input.glsl -> generated/resources/shaders/input.glsl.h
        set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/generated/resources/shaders/${SHADER_NAME}.h")
        set(VARIABLE_NAME "${SHADER_NAME_WE}_${SHADER_EXT_CLEAN}")

        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated/resources/shaders"
            COMMAND ${CMAKE_COMMAND}
                -DSHADER_FILE=${SHADER_FILE}
                -DOUTPUT_FILE=${OUTPUT_FILE}
                -DVARIABLE_NAME=${VARIABLE_NAME}
                -P ${CMAKE_SOURCE_DIR}/cmake/embed_shader.cmake
            DEPENDS ${SHADER_FILE}
            COMMENT "Embedding shader ${SHADER_NAME}"
            VERBATIM
        )
    endforeach()
endfunction()

# Embed a binary resource as C++ unsigned char array
# Usage: embed_binary_resource(output_path input_path)
function(embed_binary_resource output_path input_path)
    # Derive variable name from output filename
    get_filename_component(OUTPUT_NAME ${output_path} NAME_WE)
    string(REPLACE "." "_" VARIABLE_NAME ${OUTPUT_NAME})

    # Convert relative paths to absolute
    if(NOT IS_ABSOLUTE ${input_path})
        set(input_path "${CMAKE_SOURCE_DIR}/${input_path}")
    endif()
    if(NOT IS_ABSOLUTE ${output_path})
        set(output_path "${CMAKE_BINARY_DIR}/${output_path}")
    endif()

    add_custom_command(
        OUTPUT ${output_path}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated/resources"
        COMMAND ${CMAKE_COMMAND}
            -DBINARY_FILE=${input_path}
            -DOUTPUT_FILE=${output_path}
            -DVARIABLE_NAME=${VARIABLE_NAME}
            -P ${CMAKE_SOURCE_DIR}/cmake/embed_binary.cmake
        DEPENDS ${input_path}
        COMMENT "Embedding resource ${VARIABLE_NAME}"
        VERBATIM
    )
endfunction()