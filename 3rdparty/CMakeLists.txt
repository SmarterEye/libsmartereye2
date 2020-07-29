# logger
include(${CMAKE_CURRENT_LIST_DIR}/easyloggingpp/CMakeLists.txt)
set(LOG_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/easyloggingpp/src)

# json
set(JSON_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/json)

# usb
include(${PROJECT_SOURCE_DIR}/cmake/libusb_config.cmake)
set(USB_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/libusb/libusb)

# net
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libuv)
set(UV_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/libuv/include)
