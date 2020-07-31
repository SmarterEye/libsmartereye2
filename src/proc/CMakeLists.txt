target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/processing.cc"
        "${CMAKE_CURRENT_LIST_DIR}/filter.cc"
        "${CMAKE_CURRENT_LIST_DIR}/synthetic_stream.cc"
        "${CMAKE_CURRENT_LIST_DIR}/syncer_process.cc"

        "${CMAKE_CURRENT_LIST_DIR}/processing.h"
        "${CMAKE_CURRENT_LIST_DIR}/filter.h"
        "${CMAKE_CURRENT_LIST_DIR}/synthetic_stream.h"
        "${CMAKE_CURRENT_LIST_DIR}/syncer_process.h"
        )