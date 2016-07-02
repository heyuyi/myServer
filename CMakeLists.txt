cmake_minimum_required(VERSION 3.5)
project(myServer)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

set(SOURCE_FILES source/main.c source/io_func.c source/serve.c source/web_serve.c source/web_serve.c)
add_executable(myServer ${SOURCE_FILES})
target_link_libraries(myServer pthread)