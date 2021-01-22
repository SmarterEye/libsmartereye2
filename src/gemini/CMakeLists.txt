target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/gemini_device.cc"
        "${CMAKE_CURRENT_LIST_DIR}/gemini_info.cc"
        "${CMAKE_CURRENT_LIST_DIR}/gemini_sensor.cc"
        "${CMAKE_CURRENT_LIST_DIR}/gemini_serial_port.cc"

        "${CMAKE_CURRENT_LIST_DIR}/gemini_device.h"
        "${CMAKE_CURRENT_LIST_DIR}/gemini_info.h"
        "${CMAKE_CURRENT_LIST_DIR}/gemini_sensor.h"
        "${CMAKE_CURRENT_LIST_DIR}/gemini_serial_port.h"
        "${CMAKE_CURRENT_LIST_DIR}/tlv_data.h"
        )