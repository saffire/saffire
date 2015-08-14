
include(ExternalProject)
include(ExternalTools)

set(CPUID_INSTALL_PATH ${CMAKE_BINARY_DIR}/lib/libcpuid)

externalproject_add(
    cpuid_externalproject
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/libcpuid
    CONFIGURE_COMMAND
        COMMAND ${LIBTOOLIZE}
        COMMAND ${AUTORECONF} --install
    BUILD_COMMAND
        COMMAND ./configure --prefix=${CPUID_INSTALL_PATH}
        COMMAND ${MAKE}
        COMMAND ${MAKE} install
    BUILD_IN_SOURCE 1
)

add_library(cpuid STATIC IMPORTED)
add_dependencies(cpuid cpuid_externalproject)
set_target_properties(cpuid PROPERTIES IMPORTED_LOCATION ${CPUID_INSTALL_PATH}/lib/libcpuid.a)
include_directories(${CPUID_INSTALL_PATH}/include)
