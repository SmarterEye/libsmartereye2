cmake_minimum_required(VERSION 3.11.0)
project(libsmartereye2 LANGUAGES CXX C)

include(cmake/options.cmake)
include(cmake/common_config.cmake)
include(cmake/version_config.cmake)

# macro definition located at "cmake/common_config.cmake"
config_cxx_flags()

if(BUILD_SHARED_LIBS)
    add_library(${LSE2_TARGET} SHARED "")
else()
    add_library(${LSE2_TARGET} STATIC "")
endif()

include(src/CMakeLists.txt)
include(3rdparty/CMakeLists.txt)

add_dependencies(${LSE2_TARGET} usb)
add_dependencies(${LSE2_TARGET} uv)

target_link_libraries(${LSE2_TARGET} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${LSE2_TARGET}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        src
        ${LOG_INC_DIR}
        ${JSON_INC_DIR}
        ${USB_INC_DIR}
        ${UV_INC_DIR}
        )

# set library version
set_target_properties(${LSE2_TARGET} PROPERTIES VERSION ${SMARTEREYE_VERSION_STRING} SOVERSION "${SMARTEREYE_VERSION_MAJOR}.${SMARTEREYE_VERSION_MINOR}")

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()
