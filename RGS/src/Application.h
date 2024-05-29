#pragma once 
#include "RGS.h"
#include <memory>
#include <chrono>

namespace RGS {

    struct Camera
    {
        Vec4 Pos = { 0.0f, 0.0f, 0.0f, 1.0f };
        Vec4 Up = { 0.0f, 1.0f, 0.0f, 0.0f };
        Vec4 Right = { 1.0f, 0.0f, 0.0f, 0.0f };
        Vec4 Dir = { 0.0f, 0.0f, -1.0f, 0.0f };

        Mat4 ViewMat4;
        Mat4 ProjectionMat4;
    };

    class Application
    {
    public:
        Application(const char* name, const uint32_t width, const uint32_t height);
        ~Application();

        void Run();

    private:
        void Init();
        void Terminate();

        void OnUpdate(float t);

        float GetDeltaTime();

        void DrawSquare(const Vec3& pos, const float rotateAngle, const float scaleWidth, const float scaleHeight);
        void DrawModel(const Vec3& pos, const float rotateAngle, const float scaleWidth, const float scaleHeight);
        void DrawTextureModel(const Vec3& pos, const float rotateAngle, const float scaleWidth, const float scaleHeight);

    private:
        const char* m_Name;
        uint32_t m_Width;
        uint32_t m_Height;
        Camera m_Camera;
        std::chrono::steady_clock::time_point m_LastFrameTime;

        float m_RotateAngle = 0.0f;
        std::unique_ptr<BlinnModel> m_Model;

        std::unique_ptr<Framebuffer> m_Framebuffer;
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<Texture> m_Diffuse;
        std::unique_ptr<Texture> m_Specular;
    };
}