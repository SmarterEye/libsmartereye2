target_sources(${LSE2_TARGET}
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/se_common.cc"
        "${CMAKE_CURRENT_LIST_DIR}/se_common.h"
        )

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/concurrency)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/core)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/device)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/mock)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/net)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pipeline)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/proc)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/sensor)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/streaming)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/usb)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/gemini)
