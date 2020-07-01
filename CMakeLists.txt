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

target_link_libraries(${LSE2_TARGET} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
include_directories(${LSE2_TARGET} src 3rdparty/easyloggingpp/src)
target_include_directories(${LSE2_TARGET}
#        PRIVATE
#        ${LIBUSB_LOCAL_INCLUDE_PATH}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
#        PRIVATE ${USB_INCLUDE_DIRS}
)

# set library version
#set_target_properties(${LSE2_TARGET} PROPERTIES FOLDER Library)
set_target_properties(${LSE2_TARGET} PROPERTIES VERSION ${SMARTEREYE_VERSION_STRING} SOVERSION "${SMARTEREYE_VERSION_MAJOR}.${SMARTEREYE_VERSION_MINOR}")

include(src/CMakeLists.txt)
include(3rdparty/CMakeLists.txt)
include(cmake/libusb_config.cmake)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()
