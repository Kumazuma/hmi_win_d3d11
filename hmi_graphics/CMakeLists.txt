cmake_minimum_required(VERSION 3.29)
project(hmi_graphics)

set(CMAKE_CXX_STANDARD 14)

add_library(hmi_graphics SHARED
        src/graphics_element.cpp
        src/graphics_system.cpp
        src/graphics_system_d3d11.cpp)
target_compile_definitions(hmi_graphics PRIVATE HMI_GRAPHICS_DLL)
target_include_directories(hmi_graphics PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/graphics)
target_include_directories(hmi_graphics INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(hmi_graphics PRIVATE d3d11.lib d2d1.lib dxgi.lib)
target_compile_definitions(hmi_graphics PUBLIC -D_WIN32_WINNT=_WIN32_WINNT_WIN8)
