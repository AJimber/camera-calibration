cmake_minimum_required(VERSION 3.10)
project(calibrate VERSION 1.7.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 4.6 REQUIRED)

add_executable(calibrate
    calibrate.cpp
    common_code.cpp
    common_code.hpp
)

add_executable(undistort
    undistort.cpp
    common_code.cpp
    common_code.hpp
)

target_include_directories(calibrate PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(calibrate PRIVATE ${OpenCV_LIBS})

target_include_directories(undistort PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(undistort PRIVATE ${OpenCV_LIBS})