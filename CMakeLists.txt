cmake_minimum_required(VERSION 3.11.0)
project(libsmartereye2 LANGUAGES CXX C)

include(cmake/options.cmake)
include(cmake/common_config.cmake)
include(cmake/version_config.cmake)

# macro definition located at "cmake/common_config.cmake"
config_cxx_flags()

set(CMAKE_DEBUG_POSTFIX d)

if(BUILD_SHARED_LIBS)
    add_library(${LSE2_TARGET} SHARED "")
else()
    add_library(${LSE2_TARGET} STATIC "")
endif()

include(3rdparty/CMakeLists.txt)
include(include/CMakeLists.txt)
include(src/CMakeLists.txt)

add_dependencies(${LSE2_TARGET} usb serial)

target_link_libraries(${LSE2_TARGET}
        PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        usb
        serial
        )
target_include_directories(${LSE2_TARGET}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        include/smartereye2
        src
        ${LOG_INC_DIR}
        ${JSON_INC_DIR}
        ${USB_INC_DIR}
        ${SERIAL_INC_DIR}
        )

set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/_output")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/bin)

set_target_properties(${LSE2_TARGET}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/bin
        VERSION ${SMARTEREYE_VERSION_STRING}
        SOVERSION "${SMARTEREYE_VERSION_MAJOR}.${SMARTEREYE_VERSION_MINOR}")

include_directories(include/smartereye2 src)
link_directories(${CMAKE_CURRENT_BINARY_DIR})

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()
