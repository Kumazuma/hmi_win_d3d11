cmake_minimum_required(VERSION 3.29)
project(hmi_system)

set(CMAKE_CXX_STANDARD 14)

add_executable(hmi_system WIN32 src/main.cpp)
target_link_libraries(hmi_system PRIVATE hmi_graphics d2d1.lib)
add_custom_command(TARGET hmi_system POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hmi_graphics> $<TARGET_FILE_DIR:hmi_system>
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:hmi_graphics> $<TARGET_FILE_DIR:hmi_system>)
