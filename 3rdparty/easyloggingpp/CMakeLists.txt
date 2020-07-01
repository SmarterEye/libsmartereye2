# A custom config
add_definitions(-DELPP_THREAD_SAFE)
add_definitions(-DELPP_NO_DEFAULT_LOG_FILE)

target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src/easylogging++.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/easylogging++.h"
)
