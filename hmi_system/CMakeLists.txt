cmake_minimum_required(VERSION 3.29)
project(hmi_system)

set(CMAKE_CXX_STANDARD 14)

add_executable(hmi_system WIN32 src/main.cpp)
target_link_libraries(hmi_system PRIVATE hmi_graphics)
