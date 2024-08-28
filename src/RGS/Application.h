#pragma once 
#include "RGS/Window.h"
#include "RGS/Renderer.h"
#include "RGS/Shaders/BlinnShader.h"

#include <string>
#include <chrono>
#include <vector>

namespace RGS {
    struct Camera
    {
        Vec4 Pos = { 0.0f, 0.0f, 2.0f, 1.0f };
        Vec4 Right = { 1.0f, 0.0f, 0.0f, 0.0f };
        Vec4 Up = { 0.0f, 1.0f, 0.0f, 0.0f };
        Vec4 Dir = { 0.0f, 0.0f, -1.0f, 0.0f };
        float Aspect = 4.0f / 3.0f;
    };

    class Application
    {
    public:
        Application(const std::string name, const int width, const int height);
        ~Application();

        void Run();
    private:

        void Init();
        void Terminate();

        void OnCameraUpdate(float time);
        void OnUpdate(float time);

        void LoadMesh(const char* fileName);
    private:
        std::string m_Name;
        int m_Width, m_Height;
        std::chrono::steady_clock::time_point m_LastFrameTime;

        Window* m_Window;
        Camera m_Camera;

        std::vector<Triangle<BlinnVertex>> m_Mesh;

        BlinnUniforms m_Uniforms;
    };
} 