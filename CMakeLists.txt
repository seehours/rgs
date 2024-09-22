# 要求最低Cmake版本
cmake_minimum_required(VERSION 3.15)
# 项目名称
project(RGS LANGUAGES CXX)
# C++标准
set(CMAKE_CXX_STANDARD 17)

# =========================================
# ================== RGS ==================
# =========================================
set(HEADERS
    "RGS/src/rgspch.h"
    "RGS/src/Application.h"

    "RGS/src/RGS/Base/Base.h"
    "RGS/src/RGS/Base/Maths.h"
    "RGS/src/RGS/Base/Instrumentor.h"

    "RGS/src/RGS/InputCode.h"
    "RGS/src/RGS/Window.h"
    "RGS/src/RGS/Platform.h"
    "RGS/src/RGS/Texture.h"
    "RGS/src/RGS/JobSystem.h"
    "RGS/src/RGS/Timer.h"
    "RGS/src/RGS/Config.h"

    "RGS/src/RGS/Layer/Layer.h"
    "RGS/src/RGS/Layer/CameraLayer.h"
    "RGS/src/RGS/Layer/IBLPBRLayer.h"
    "RGS/src/RGS/Layer/ToolLayer.h"

    "RGS/src/RGS/Render/Mesh.h"
    "RGS/src/RGS/Render/Pipeline.h"
    "RGS/src/RGS/Render/Renderer.h"
    "RGS/src/RGS/Render/RenderCommand.h"
    "RGS/src/RGS/Render/MSAASettings.h"

    "RGS/src/RGS/Shader/ShaderBase.h"
    "RGS/src/RGS/Shader/SkyboxShader.h"
    "RGS/src/RGS/Shader/ConvSkyShader.h"
    "RGS/src/RGS/Shader/BRDFShader.h" 
    "RGS/src/RGS/Shader/IBLPBRShader.h"
    "RGS/src/RGS/Shader/BlinnShader.h"
    "RGS/src/RGS/Shader/PBRShader.h"
    "RGS/src/RGS/Shader/FlatColorShader.h"
    
    "RGS/src/ImGui/ImGuiWindow.h"
    "RGS/src/Windows/WindowsWindow.h"
)

set(RGS_SOURCES 
    "RGS/src/rgspch.cpp"
    "RGS/src/Application.cpp"
    "RGS/src/RGS/Base/Maths.cpp"

    "RGS/src/RGS/Platform.cpp"
    "RGS/src/RGS/Window.cpp"
    "RGS/src/RGS/Texture.cpp"
    "RGS/src/RGS/JobSystem.cpp"
    "RGS/src/RGS/Timer.cpp"

    "RGS/src/RGS/Layer/Layer.cpp"
    "RGS/src/RGS/Layer/CameraLayer.cpp"
    "RGS/src/RGS/Layer/IBLPBRLayer.cpp"
    "RGS/src/RGS/Layer/ToolLayer.cpp"

    "RGS/src/RGS/Render/Framebuffer.cpp"
    "RGS/src/RGS/Render/RenderCommand.cpp"
    "RGS/src/RGS/Render/Renderer.cpp"
    "RGS/src/RGS/Render/Pipeline.cpp"

    "RGS/src/RGS/Shader/SkyboxShader.cpp" 
    "RGS/src/RGS/Shader/ConvSkyShader.cpp"
    "RGS/src/RGS/Shader/BRDFShader.cpp" 
    "RGS/src/RGS/Shader/BlinnShader.cpp" 
    "RGS/src/RGS/Shader/IBLPBRShader.cpp"
    "RGS/src/RGS/Shader/PBRShader.cpp"
    "RGS/src/RGS/Shader/FlatColorShader.cpp"

    "RGS/src/stb/stb_image.cpp"
    "RGS/src/stb/stb_image_write.cpp"
    "RGS/src/stb/stb_image_resize2.cpp"
    
    "RGS/src/ImGui/imgui_stdlib.cpp"
    "RGS/src/ImGui/ImGuiWindow.cpp" 

    "RGS/src/Windows/WindowsWindow.cpp"

)

# =========================================
# ================= ImGui =================
# =========================================
file(GLOB IMGUI_SOURCES RGS/vendor/imgui/*.cpp)
set(IMGUI_SOURCES ${IMGUI_SOURCES}
    "RGS/vendor/imgui/backends/imgui_impl_win32.cpp"
    "RGS/vendor/imgui/backends/imgui_impl_dx11.cpp")
set(IMGUI_LINK_LIBRARY "d3d11.lib" "d3dcompiler.lib" "dxgi.lib")

set(INCLUDE_PATH 
    "RGS/vendor/stb_image"
    "RGS/vendor/imgui"
    "RGS/src"
)

set(TARGET "RGS")

add_executable(${TARGET} ${RGS_SOURCES} ${IMGUI_SOURCES} ${HEADERS} "RGS/src/Main.cpp")
target_include_directories(${TARGET} PRIVATE ${INCLUDE_PATH})
target_link_libraries(${TARGET} ${IMGUI_LINK_LIBRARY})

target_precompile_headers(${TARGET} PRIVATE "RGS/src/rgspch.h")

#判断
# Debug 配置
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(${TARGET} PRIVATE RGS_BUILD_DEBUG)
# Release 配置
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(${TARGET} PRIVATE RGS_BUILD_RELEASE)
# 其他配置
elseif (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    target_compile_definitions(${TARGET} PRIVATE RGS_BUILD_RELEASE)
endif ()

set(ASSETS_SRC "${CMAKE_SOURCE_DIR}/Assets")
set(ASSETS_DST "$<TARGET_FILE_DIR:${TARGET}>/Assets")

set(ASSETS_CMD copy_directory)
set(ASSETS_MSG "[RGS] [post build] Copy Assets")

add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo
    "${ASSETS_MSG}: ${ASSETS_SRC} to ${ASSETS_DST}")

add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E ${ASSETS_CMD}
    "${ASSETS_SRC}" "${ASSETS_DST}")

# 设置启动项目
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${TARGET})
