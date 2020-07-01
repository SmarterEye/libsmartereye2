include(ExternalProject)

ExternalProject_Add(
    libusb

    GIT_REPOSITORY "https://github.com/libusb/libusb.git"
    GIT_TAG "e782eeb2514266f6738e242cdcb18e3ae1ed06fa" # "v1.0.23" + Mac get_device_list hang fix

    UPDATE_COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libusb/CMakeLists.txt
            ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libusb/CMakeLists.txt
    PATCH_COMMAND ""

    SOURCE_DIR "3rdparty/libusb/"
    CMAKE_ARGS -DCMAKE_CXX_STANDARD_LIBRARIES=${CMAKE_CXX_STANDARD_LIBRARIES}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libusb_install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/libusb_install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}usb${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(usb INTERFACE)
target_include_directories(usb INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/3rdparty/libusb/libusb>)
target_link_libraries(usb INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/libusb_install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}usb${CMAKE_STATIC_LIBRARY_SUFFIX})
set(USE_EXTERNAL_USB ON) # INTERFACE libraries can't have real deps, so targets that link with usb need to also depend on libusb

if (APPLE)
  find_library(corefoundation_lib CoreFoundation)
  find_library(iokit_lib IOKit)
  target_link_libraries(usb INTERFACE objc ${corefoundation_lib} ${iokit_lib})
endif()
