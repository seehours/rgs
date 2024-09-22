#include "CameraLayer.h"

#include "Application.h"
#include "RGS/Render/Framebuffer.h"

#include <imgui.h>

namespace RGS {

    CameraLayer* CameraLayer::s_Instance = nullptr;

    void CameraLayer::OnAttach()
    {
        Framebuffer& framebuffer = Application::Instance().GetFramebuffer();
        float width = framebuffer.GetWidth();
        float height = framebuffer.GetHeight();
        m_Camera.Aspect = width / height;
        m_Camera.Pos = { 0.0f, 0.0f, 2.0f, 1.0f };
    }

    void CameraLayer::OnDetach() {}

    void CameraLayer::OnUpdate(float t) 
    {
        Window& window = Application::Instance().GetWindow();
        constexpr float speed = 1.0f;
        if (window.GetKey(RGS_KEY_SPACE) == RGS_PRESS)
            m_Camera.Pos += speed * t * m_Camera.Up;
        if (window.GetKey(RGS_KEY_LEFT_SHIFT) == RGS_PRESS)
            m_Camera.Pos -= speed * t * m_Camera.Up;
        if (window.GetKey(RGS_KEY_D) == RGS_PRESS)
            m_Camera.Pos += speed * t * m_Camera.Right;
        if (window.GetKey(RGS_KEY_A) == RGS_PRESS)
            m_Camera.Pos -= speed * t * m_Camera.Right;
        if (window.GetKey(RGS_KEY_W) == RGS_PRESS)
            m_Camera.Pos += speed * t * m_Camera.Dir;
        if (window.GetKey(RGS_KEY_S) == RGS_PRESS)
            m_Camera.Pos -= speed * t * m_Camera.Dir;

        constexpr float rotateSpeed = 1.0f;
        Mat4 rotation = Mat4Identity();
        if (window.GetKey(RGS_KEY_Q) == RGS_PRESS)
            rotation = Mat4RotateY(t * rotateSpeed);
        if (window.GetKey(RGS_KEY_E) == RGS_PRESS)
            rotation = Mat4RotateY(-t * rotateSpeed);
        m_Camera.Dir = rotation * m_Camera.Dir;
        m_Camera.Dir = { Normalize(m_Camera.Dir), 0.0f };
        m_Camera.Right = rotation * m_Camera.Right;
        m_Camera.Right = { Normalize(m_Camera.Right), 0.0f };

    }
    
    void CameraLayer::OnImGuiRender(float t)
    {
        ImGui::Begin(m_DebugName.c_str());
        ImGui::DragFloat("Camera Aspect", (float*)&m_Camera.Aspect, 0.1f, EPSILON, 8);
        ImGui::DragFloat3("Camera Pos", (float*)&m_Camera.Pos, 0.1f);
        ImGui::DragFloat3("Camera Dir", (float*)&m_Camera.Dir, 0.1f);
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Total Faces: %d", Renderer::s_FaceCount);
        ImGui::End();
    }

}