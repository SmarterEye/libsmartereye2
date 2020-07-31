target_include_directories(${LSE2_TARGET}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/uv-cpp
        ${CMAKE_CURRENT_LIST_DIR}/satp-cpp
        )

add_subdirectory(uv-cpp)
add_subdirectory(satp)
