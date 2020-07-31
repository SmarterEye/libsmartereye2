CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(uv_cpp)

list(APPEND uv_cpp_source
        ${CMAKE_CURRENT_LIST_DIR}/async.cc
        ${CMAKE_CURRENT_LIST_DIR}/async.h
        )

ADD_LIBRARY(uv_cpp STATIC ${uv_cpp_source})

add_dependencies(uv_cpp uv)

target_include_directories(uv_cpp
        PUBLIC
        ${UV_INC_DIR}
        )

target_link_directories(uv_cpp PUBLIC uv})
