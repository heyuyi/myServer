cmake_minimum_required(VERSION 3.5)
project(dataView)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

set(SOURCE_FILES main.c ../../source/io_func.c)
set(EXECUTABLE_OUTPUT_PATH "../")
add_executable(data_view ${SOURCE_FILES})
