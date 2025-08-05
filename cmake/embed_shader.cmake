file(READ ${SHADER_FILE} SHADER_CONTENT)
file(WRITE ${OUTPUT_FILE}
    "#pragma once\n"
    "inline constexpr const char* ${VARIABLE_NAME} = R\"SHADER(\n"
    "${SHADER_CONTENT}"
    ")SHADER\";\n")
