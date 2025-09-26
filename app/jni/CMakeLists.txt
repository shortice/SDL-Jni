cmake_minimum_required(VERSION 3.6)

project(JniSample LANGUAGES CXX C)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_program(CCACHE_EXECUTABLE ccache)

if(CCACHE_EXECUTABLE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
    set(CMAKE_C_COMPILER_LAUNCHER ccache)
endif()

add_subdirectory(SDL)

set(imgui_source
    ${CMAKE_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp
    ${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_sdlrenderer3.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_demo.cpp
)

add_library(
    main SHARED
    ${CMAKE_SOURCE_DIR}/main.cpp
    ${CMAKE_SOURCE_DIR}/android_jni.cpp
    ${imgui_source}
) # Need for SDL android build

target_include_directories(
    main PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
)

target_link_libraries(main
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL3::SDL3main>
    $<IF:$<TARGET_EXISTS:SDL3::SDL3>,SDL3::SDL3,SDL3::SDL3-static>
)
