target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/backend.cc"
        "${CMAKE_CURRENT_LIST_DIR}/context.cc"
        "${CMAKE_CURRENT_LIST_DIR}/device.cc"
        "${CMAKE_CURRENT_LIST_DIR}/calibrated_device.cc"
        "${CMAKE_CURRENT_LIST_DIR}/update_device.cc"
        "${CMAKE_CURRENT_LIST_DIR}/device_hub.cc"

        "${CMAKE_CURRENT_LIST_DIR}/backend.h"
        "${CMAKE_CURRENT_LIST_DIR}/context.h"
        "${CMAKE_CURRENT_LIST_DIR}/device.h"
        "${CMAKE_CURRENT_LIST_DIR}/calibrated_device.h"
        "${CMAKE_CURRENT_LIST_DIR}/update_device.h"
        "${CMAKE_CURRENT_LIST_DIR}/device_hub.h"
        "${CMAKE_CURRENT_LIST_DIR}/device_info.h"
        )