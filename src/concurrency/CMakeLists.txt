target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dispatcher.cc"

        "${CMAKE_CURRENT_LIST_DIR}/concurrency.h"
        "${CMAKE_CURRENT_LIST_DIR}/consumer_queue.h"
        "${CMAKE_CURRENT_LIST_DIR}/dispatcher.h"
        "${CMAKE_CURRENT_LIST_DIR}/watchdog.h"
        )