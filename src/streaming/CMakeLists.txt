target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/streaming.cc"
        "${CMAKE_CURRENT_LIST_DIR}/stream_profile.cc"

        "${CMAKE_CURRENT_LIST_DIR}/streaming.h"
        "${CMAKE_CURRENT_LIST_DIR}/stream_profile.h"
        "${CMAKE_CURRENT_LIST_DIR}/serialization.h"
        )