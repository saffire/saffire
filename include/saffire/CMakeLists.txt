########################################################################
# Generate custom header files
########################################################################

# config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

# Generate gitversion.h
execute_process(
    COMMAND           git rev-parse HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE   GITREV_SHA1
    OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gitversion.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/gitversion.h" @ONLY)
