target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/se_global.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/se_callbacks.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/se_types.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/se_util.hpp"

        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/alg/opense_basic_type.h"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/alg/opense_camera.h"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/alg/opense_vehicle.h"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/alg/packed_types.h"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/alg/algorithmresult.h"

        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/core/core_types.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/core/frame.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/core/frame_set.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/core/options.hpp"

        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/device/context.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/device/device.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/device/device_list.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/device/calibrated_device.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/device/record_playback.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/device/update_device.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/device/device_types.hpp"

        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/pipeline/pipeline.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/pipeline/pipeline_config.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/pipeline/pipeline_profile.hpp"

        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/proc/processing.hpp"

        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/sensor/sensor.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/sensor/notification.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/sensor/sensor_types.hpp"

        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/streaming/stream_profile.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/smartereye2/streaming/stream_types.hpp"
        )