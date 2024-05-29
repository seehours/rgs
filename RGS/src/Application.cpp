#pragma once 
#include "Application.h"

#include <memory>
#include <iostream>
#include <algorithm>
#include <chrono>

class Timer
{
public:
    Timer()
    {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~Timer()
    {
        Stop();
    }

    void Stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        auto start = std::chrono::time_point_cast<std::chrono::microseconds> (m_StartTimepoint).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds> (endTimepoint).time_since_epoch().count();

        auto duration = end - start;
        double ms = duration * 0.001;

        std::cout << duration << "us (" << ms << "ms)\n";
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
};

namespace RGS {

    Application::Application(const char* name, const uint32_t width, const uint32_t height)
        :m_Name(name), m_Width(width), m_Height(height)
    {
        Init();
    }
    Application::~Application()
    {
        Terminate();
    }

    void Application::Init()
    {
        Platform::Init();
        m_Window.reset(Window::Create(m_Name, m_Width, m_Height));
        m_Framebuffer.reset(new Framebuffer(m_Width, m_Height));
        float aspect = (float)m_Width / (float)m_Height;
        m_Camera.ProjectionMat4 = Mat4Perspective(90.0f / 360.0f * 2.0f * 3.1415926f, aspect, 0.1f, 100.0f);
        m_Camera.ViewMat4 = Mat4Identity();
        m_LastFrameTime = std::chrono::high_resolution_clock::now();
        m_Model.reset(new BlinnModel("Assets\\box.obj"));

        m_Diffuse.reset(new Texture("Assets\\container2.png"));
        m_Specular.reset(new Texture("Assets\\container2_specular.png"));
    }
    void Application::Terminate()
    {
        Platform::Terminate();
    }

    float Application::GetDeltaTime()
    {
        auto nowFrameTime = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds> (m_LastFrameTime).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds> (nowFrameTime).time_since_epoch().count();
        auto duration = end - start;
        float deltaTime = duration * 0.001f * 0.001f;
        m_LastFrameTime = nowFrameTime;

        return deltaTime;
    }

    void Application::Run()
    {
        // 消息循环
        while (!m_Window->Closed())
        {
            Timer t;
            m_Framebuffer->Clear({ 0.125f, 0.125f, 0.2f });
            m_Framebuffer->ClearDepth(1.0f);

            Platform::PollInputEvents();
            float deltaTime = GetDeltaTime();
            OnUpdate(deltaTime);

            m_Window->DrawFramebuffer(*m_Framebuffer);
        }
    }

    void Application::OnUpdate(float t)
    {
        //Timer timer;
        if (m_Window != nullptr)
        {
            constexpr float speed = 1.0f;
            if (m_Window->GetKey(RGS_KEY_SPACE) == RGS_PRESS)
                m_Camera.Pos += speed * t * m_Camera.Up;
            if (m_Window->GetKey(RGS_KEY_LEFT_SHIFT) == RGS_PRESS)
                m_Camera.Pos -= speed * t * m_Camera.Up;
            if (m_Window->GetKey(RGS_KEY_D) == RGS_PRESS)
                m_Camera.Pos += speed * t * m_Camera.Right;
            if (m_Window->GetKey(RGS_KEY_A) == RGS_PRESS)
                m_Camera.Pos -= speed * t * m_Camera.Right;
            if (m_Window->GetKey(RGS_KEY_W) == RGS_PRESS)
                m_Camera.Pos += speed * t * m_Camera.Dir;
            if (m_Window->GetKey(RGS_KEY_S) == RGS_PRESS)
                m_Camera.Pos -= speed * t * m_Camera.Dir;

            constexpr float rotateSpeed = 1.0f;
            Mat4 rotation = Mat4Identity();
            if (m_Window->GetKey(RGS_KEY_Q) == RGS_PRESS)
                rotation = Mat4RotateY(t * rotateSpeed);
            if (m_Window->GetKey(RGS_KEY_E) == RGS_PRESS)
                rotation = Mat4RotateY(- t * rotateSpeed);
            m_Camera.Dir = rotation * m_Camera.Dir;
            m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
            m_Camera.Right = rotation * m_Camera.Right;
            m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };

            Mat4 lookAt = Mat4LookAt(m_Camera.Pos, m_Camera.Pos + m_Camera.Dir, m_Camera.Up);
            m_Camera.ViewMat4 = lookAt;
        }

        m_RotateAngle += t;
        //DrawSquare({ 0.0f, 0.0f, -2.0f }, m_RotateAngle, 1.0f, 1.0f);
        DrawTextureModel({ 0.0f, 0.0f, -1.0f }, 0.0f, 1.0f, 1.0f);
    }

    void Application::DrawSquare(const Vec3& pos, const float rotateAngle, const float scaleWidth, const float scaleHeight)
    {
        Vec4 triVertex[4];
        triVertex[0] = { -0.5f, 0.5f, 0.0f, 1.0f };
        triVertex[1] = { -0.5f, -0.5f, 0.0f, 1.0f };
        triVertex[2] = { 0.5f, -0.5f, 0.0f, 1.0f };
        triVertex[3] = { 0.5f, 0.5f, 0.0f, 1.0f };

        Program program(BlinnVertexShader, BlinnFragmentShader);
        program.EnableDoubleSided = true;

        BlinnUniforms uniforms;
        uniforms.CameraPos = m_Camera.Pos;
        uniforms.LightPos = { 0.0f, 0.0f, 0.0f };
        Mat4 scale = Mat4Scale(scaleWidth, scaleHeight, 1.0f);
        Mat4 trans = Mat4Translate(pos.X, pos.Y, pos.Z);
        Mat4 rotate = Mat4RotateY(rotateAngle);
        Mat4 model = trans * rotate * scale;
        uniforms.Model = model;
        uniforms.MVP = m_Camera.ProjectionMat4 * m_Camera.ViewMat4 * model;
        uniforms.ModelNormalToWorld = rotate;

        Triangle<BlinnVertex> triangle;
        for (int i = 0; i < 2; i++)
        {
            int index0 = 0;
            int index1 = i + 1;
            int index2 = i + 2;
            triangle[0].ModelPos = triVertex[index0];
            triangle[1].ModelPos = triVertex[index1];
            triangle[2].ModelPos = triVertex[index2];
            triangle[0].ModelNormal = { 0.0f ,0.0f ,1.0f };
            triangle[1].ModelNormal = { 0.0f ,0.0f ,1.0f };
            triangle[2].ModelNormal = { 0.0f ,0.0f ,1.0f };

            Renderer::Draw(*m_Framebuffer, program, triangle, uniforms);
        }
    }
    
    void Application::DrawModel(const Vec3& pos, const float rotateAngle, const float scaleWidth, const float scaleHeight)
    {
        BlinnUniforms uniforms;
        uniforms.CameraPos = m_Camera.Pos;
        uniforms.LightPos = { -1.0f, 1.0f, 0.0f };
        Mat4 scale = Mat4Scale(scaleWidth, scaleHeight, 1.0f);
        Mat4 trans = Mat4Translate(pos.X, pos.Y, pos.Z);
        Mat4 rotate = Mat4RotateY(rotateAngle);
        Mat4 model = trans * rotate * scale;

        uniforms.Model = model;
        uniforms.MVP = m_Camera.ProjectionMat4 * m_Camera.ViewMat4 * model;
        uniforms.ModelNormalToWorld = rotate;

        m_Model->SetUniforms(uniforms);
        m_Model->Draw(*m_Framebuffer);
    }

    void Application::DrawTextureModel(const Vec3& pos, const float rotateAngle, const float scaleWidth, const float scaleHeight)
    {
        BlinnUniforms uniforms;
        uniforms.CameraPos = m_Camera.Pos;
        uniforms.LightPos = { -1.0f, 1.0f, 0.0f };
        uniforms.Diffuse = m_Diffuse.get();
        uniforms.Specular = m_Specular.get();
        Mat4 scale = Mat4Scale(scaleWidth, scaleHeight, 1.0f);
        Mat4 trans = Mat4Translate(pos.X, pos.Y, pos.Z);
        Mat4 rotate = Mat4RotateY(rotateAngle);
        Mat4 model = trans * rotate * scale;

        uniforms.Model = model;
        uniforms.MVP = m_Camera.ProjectionMat4 * m_Camera.ViewMat4 * model;
        uniforms.ModelNormalToWorld = rotate;

        m_Model->SetUniforms(uniforms);
        m_Model->Draw(*m_Framebuffer);
    }


}