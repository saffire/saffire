cmake_minimum_required(VERSION 2.8.3)

project(saffire)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

IF (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    IF (CMAKE_COMPILER_IS_GNUCXX)
        include(CodeCoverage)
        setup_target_for_coverage(saffire_coverage "${CMAKE_SOURCE_DIR}/unittest.sh" coverage)

        #target_link_libraries(${PROJECT_TEST_NAME} gcov)
    ENDIF()
ENDIF()

set(saffire_VERSION_MAJOR 0)
set(saffire_VERSION_MINOR 1)

add_subdirectory(include/saffire)
add_subdirectory(src)
add_subdirectory(unittests/core)
