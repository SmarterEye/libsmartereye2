cmake_minimum_required(VERSION 3.1.0)

project(SmartereyeExamplesHelloSmartereye)

if (WIN32)
    set(OpenCV_DIR "D:/Program Files/opencv_341/build/install/x64/vc14/lib")
else ()
    set(OpenCV_DIR "/path/to/your/opencv/cmake/or/nothing")
endif ()
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(hello_smartereye hello_smartereye.cc)
set_property(TARGET hello_smartereye PROPERTY CXX_STANDARD 11)
target_link_libraries(hello_smartereye ${DEPENDENCIES} ${OpenCV_LIBS})
include_directories(hello_smartereye)
set_target_properties(hello_smartereye PROPERTIES FOLDER "examples")

install(TARGETS hello_smartereye RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})