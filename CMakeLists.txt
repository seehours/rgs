# 要求最低Cmake版本
cmake_minimum_required(VERSION 3.15)
# 项目名称
project(RGS LANGUAGES CXX)
# C++标准
set(CMAKE_CXX_STANDARD 17)

set(HEADERS
    "RGS/src/Application.h"
    "RGS/src/Base.h"
    "RGS/src/Framebuffer.h"
    "RGS/src/InputCode.h"
    "RGS/src/Layouts.h"
    "RGS/src/Model.h"
    "RGS/src/Math.h"    
    "RGS/src/Window.h"
    "RGS/src/Platform.h"
    "RGS/src/Renderer.h"
    "RGS/src/RGS.h"
    "RGS/src/Shader.h"
    "RGS/src/Texture.h"
    "RGS/src/WindowsWindow.h" 
)

set(SOURCES 
    "RGS/src/Application.cpp"
    "RGS/src/Framebuffer.cpp"
    "RGS/src/Model.cpp"
    "RGS/src/Math.cpp"
    "RGS/src/Main.cpp"
    "RGS/src/Application.cpp"
    "RGS/src/Window.cpp"
    "RGS/src/Platform.cpp"
    "RGS/src/Shader.cpp"
    "RGS/src/stb_image.cpp"
    "RGS/src/Texture.cpp"
    "RGS/src/WindowsWindow.cpp"
)

set(TARGET "RGS")

add_executable(${TARGET} ${SOURCES})

set(INCLUDE_PATH "RGS/vendor/stb_image")

target_include_directories(${TARGET} PRIVATE ${INCLUDE_PATH})


set(ASSETS_SRC "${CMAKE_SOURCE_DIR}/Assets")
set(ASSETS_DST "$<TARGET_FILE_DIR:${TARGET}>/Assets")

set(ASSETS_CMD copy_directory)
set(ASSETS_MSG "Copy Assets")

add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo
    "${ASSETS_MSG}: ${ASSETS_SRC} to ${ASSETS_DST}"
)
add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E ${ASSETS_CMD}
    "${ASSETS_SRC}" "${ASSETS_DST}"
)


# 设置启动项目
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${TARGET})
