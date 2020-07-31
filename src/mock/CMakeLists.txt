target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/playback/playback.cc"
        "${CMAKE_CURRENT_LIST_DIR}/playback/playback_device.cc"
        "${CMAKE_CURRENT_LIST_DIR}/playback/playback_sensor.cc"

        "${CMAKE_CURRENT_LIST_DIR}/record/record.cc"
        "${CMAKE_CURRENT_LIST_DIR}/record/record_device.cc"
        "${CMAKE_CURRENT_LIST_DIR}/record/record_sensor.cc"

        "${CMAKE_CURRENT_LIST_DIR}/playback/playback.h"
        "${CMAKE_CURRENT_LIST_DIR}/playback/playback_device.h"
        "${CMAKE_CURRENT_LIST_DIR}/playback/playback_sensor.h"

        "${CMAKE_CURRENT_LIST_DIR}/record/record.h"
        "${CMAKE_CURRENT_LIST_DIR}/record/record_device.h"
        "${CMAKE_CURRENT_LIST_DIR}/record/record_sensor.h"
        )