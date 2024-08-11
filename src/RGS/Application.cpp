#include "Application.h"

#include "RGS/Window.h"
#include "RGS/Framebuffer.h"
#include "RGS/Maths.h"
#include "RGS/Shaders/BlinnShader.h"
#include "RGS/Renderer.h"

#include <iostream>
#include <string>
#include <chrono>

namespace RGS {

    Application::Application(std::string name, const int width, const int height)
        : m_Name(name), m_Height(height), m_Width(width)
    {
        Init();
    }

    Application::~Application()
    {
        Terminate();
    }

    void Application::Init()
    {
        Window::Init();
        m_Window = Window::Create(m_Name, m_Width, m_Height);
        m_LastFrameTime = std::chrono::high_resolution_clock::now();
    }

    void Application::Terminate()
    {
        delete m_Window;
        Window::Terminate();
    }

    void Application::Run()
    {
        while (!m_Window->Closed())
        {
            auto nowFrameTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(nowFrameTime - m_LastFrameTime);
            float deltaTime = duration.count() * 0.001f * 0.001f;
            m_LastFrameTime = nowFrameTime;

            OnUpdate(deltaTime);

            Window::PollInputEvents();
        }
    }

    void Application::OnCameraUpdate(float time)
    {
        constexpr float speed = 1.0f;
        if (m_Window->GetKey(RGS_KEY_SPACE) == RGS_PRESS)
            m_Camera.Pos = m_Camera.Pos + speed * time * m_Camera.Up;
        if (m_Window->GetKey(RGS_KEY_LEFT_SHIFT) == RGS_PRESS)
            m_Camera.Pos = m_Camera.Pos - speed * time * m_Camera.Up;
        if (m_Window->GetKey(RGS_KEY_D) == RGS_PRESS)
            m_Camera.Pos = m_Camera.Pos + speed * time * m_Camera.Right;
        if (m_Window->GetKey(RGS_KEY_A) == RGS_PRESS)
            m_Camera.Pos = m_Camera.Pos - speed * time * m_Camera.Right;
        if (m_Window->GetKey(RGS_KEY_W) == RGS_PRESS)
            m_Camera.Pos = m_Camera.Pos + speed * time * m_Camera.Dir;
        if (m_Window->GetKey(RGS_KEY_S) == RGS_PRESS)
            m_Camera.Pos = m_Camera.Pos - speed * time * m_Camera.Dir;

        constexpr float rotateSpeed = 1.0f;
        Mat4 rotation = Mat4Identity();
        if (m_Window->GetKey(RGS_KEY_Q) == RGS_PRESS)
            rotation = Mat4RotateY(time * rotateSpeed);
        if (m_Window->GetKey(RGS_KEY_E) == RGS_PRESS)
            rotation = Mat4RotateY(-time * rotateSpeed);
        m_Camera.Dir = rotation * m_Camera.Dir;
        m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
        m_Camera.Right = rotation * m_Camera.Right;
        m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };
    }

    void Application::OnUpdate(float time)
    {
        OnCameraUpdate(time);

        Framebuffer framebuffer(m_Width, m_Height);
        Program program(BlinnVertexShader, BlinnFragmentShader);
        program.EnableDoubleSided = true;
        BlinnUniforms uniforms;
        Triangle<BlinnVertex> tri;

        Mat4 view = Mat4LookAt(m_Camera.Pos, m_Camera.Pos + m_Camera.Dir, { 0.0f, 1.0f, 0.0f });
        Mat4 proj = Mat4Perspective(90.0f / 360.0f * 2.0f * PI, m_Camera.Aspect, 0.1f, 100.0f);

        uniforms.MVP = proj * view;

        uniforms.IsAnother = true;
        program.EnableBlend = false;
        program.EnableWriteDepth = true;
        tri[0].ModelPos = { 10.0f, +10.0f, -10.0f, 1.0f };
        tri[1].ModelPos = { -1.0f, -1.0f, -1.0f, 1.0f };
        tri[2].ModelPos = { 10.0f, -10.0f, -10.0f, 1.0f };
        Renderer::Draw(framebuffer, program, tri, uniforms);

        uniforms.IsAnother = false;
        program.EnableBlend = true;
        program.EnableWriteDepth = false;
        tri[0].ModelPos = { -10.0f, 10.0f, -10.0f, 1.0f };
        tri[1].ModelPos = { -10.0f, -10.0f, -10.0f, 1.0f };
        tri[2].ModelPos = { 1.0f, -1.0f, -1.0f, 1.0f };
        Renderer::Draw(framebuffer, program, tri, uniforms);

        m_Window->DrawFramebuffer(framebuffer);
    }
}