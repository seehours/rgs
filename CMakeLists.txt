# 要求最低Cmake版本
cmake_minimum_required(VERSION 3.15)
# 项目名称
project(RGS LANGUAGES CXX)
# C++标准
set(CMAKE_CXX_STANDARD 17)

set(HEADERS
    "src/RGS/Application.h"
    "src/RGS/Window.h" 
    "src/RGS/WindowsWindow.h" 
    "src/RGS/Base.h"
    "src/RGS/Framebuffer.h" 
    "src/RGS/Maths.h" 
    "src/RGS/Shaders/ShaderBase.h"
    "src/RGS/Shaders/BlinnShader.h"
    "src/RGS/Renderer.h"
)

set(SOURCES 
    "src/RGS/Main.cpp"
    "src/RGS/Application.cpp"
    "src/RGS/Window.cpp" 
    "src/RGS/WindowsWindow.cpp"
    "src/RGS/Framebuffer.cpp" 
    "src/RGS/Maths.cpp"
    "src/RGS/Shaders/BlinnShader.cpp"
    "src/RGS/Renderer.cpp"
)

set(INCLUDE_PATH 
    "src"
)

set(TARGET RGS)
add_executable(${TARGET} ${HEADERS} ${SOURCES})
target_include_directories(${TARGET} PRIVATE ${INCLUDE_PATH})
