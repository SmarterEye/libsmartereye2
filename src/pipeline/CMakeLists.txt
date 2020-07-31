target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/pipeline.cc"
        "${CMAKE_CURRENT_LIST_DIR}/pipeline_profile.cc"
        "${CMAKE_CURRENT_LIST_DIR}/pipeline_config.cc"

        "${CMAKE_CURRENT_LIST_DIR}/pipeline.h"
        "${CMAKE_CURRENT_LIST_DIR}/pipeline_profile.h"
        "${CMAKE_CURRENT_LIST_DIR}/pipeline_config.h"
        )