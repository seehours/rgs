# 要求最低Cmake版本
cmake_minimum_required(VERSION 3.15)
# 项目名称
project(RGS LANGUAGES CXX)
# C++标准
set(CMAKE_CXX_STANDARD 17)

set(HEADERS
    "src/Application.h"
    "src/Window.h" 
    "src/WindowsWindow.h" 
    "src/Base.h"
)

set(SOURCES 
    "src/Main.cpp"
    "src/Application.cpp"
    "src/Window.cpp" 
    "src/WindowsWindow.cpp"
)

set(TARGET RGS)
add_executable(${TARGET} ${HEADERS} ${SOURCES})
