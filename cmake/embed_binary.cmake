file(READ ${BINARY_FILE} BINARY_HEX HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," HEX_DATA "${BINARY_HEX}")
string(REGEX REPLACE ",+$" "" HEX_DATA "${HEX_DATA}")
file(WRITE ${OUTPUT_FILE}
    "#pragma once\n"
    "inline constexpr unsigned char ${VARIABLE_NAME}[] = {${HEX_DATA}};\n"
    "inline constexpr unsigned int ${VARIABLE_NAME}_size = sizeof(${VARIABLE_NAME});\n")
