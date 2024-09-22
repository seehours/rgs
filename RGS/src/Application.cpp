#include "Application.h"

#include "RGS/Platform.h"
#include "RGS/Layer/CameraLayer.h"
#include "RGS/Layer/IBLPBRLayer.h"
#include "RGS/Layer/ToolLayer.h"
#include "RGS/JobSystem.h"

#include "RGS/Base/Instrumentor.h"

#include <chrono>

namespace RGS {
    Application* Application::s_Instance = nullptr;

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
        // Static Instance
        ASSERT(!s_Instance);
        s_Instance = this;

        // JobSystem
        JobSystem::Init();

        // Window & Framebuffer
        Platform::Init();
        m_Window = Window::Create(m_Name, m_Width, m_Height);
        m_Framebuffer = Framebuffer::Create(m_Width, m_Height);

        // ImGui
        m_ImGuiWindow = &ImGuiWindow::Instance();

        // Time
        m_LastFrameTime = std::chrono::steady_clock::now();

        // LayerStack
        m_LayerStack.PushLayer(new CameraLayer("Camera Layer"));
        m_LayerStack.PushLayer(new IBLPBRLayer("IBLPBR Layer"));
        m_LayerStack.PushLayer(new ToolLayer("Tool Layer"));
    }

    void Application::Terminate()
    {
        delete m_Window;
        Platform::Terminate();
    }

    float Application::GetDeltaTime()
    {
        auto nowFrameTime = std::chrono::steady_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds> (m_LastFrameTime).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds> (nowFrameTime).time_since_epoch().count();
        auto duration = end - start;
        float deltaTime = duration * 0.001f * 0.001f;
        m_LastFrameTime = nowFrameTime;

        return deltaTime;
    }

    void Application::Run()
    {
        RGS_PROFILE_BEGIN_SESSION("Runtime", "RGSProfile-Runtime.json");
        while (!m_Window->Closed())
        {
            RGS_PROFILE_SCOPE("Frame");
            Renderer::s_FaceCount = 0;
            float deltaTime = GetDeltaTime();

            if (!m_Window->Minimized())
            {
                RGS_PROFILE_SCOPE("layer::OnUpdate");
                for (Layer* layer : m_LayerStack)
                {
                    layer->OnUpdate(deltaTime);
                }
            }
            
            {
                RGS_PROFILE_SCOPE("layer::OnImGuiRender");

                m_ImGuiWindow->Begin();
                for (Layer* layer : m_LayerStack)
                {
                    layer->OnImGuiRender(deltaTime);
                }
                m_ImGuiWindow->End();
            }

            {
                RGS_PROFILE_SCOPE("m_Window->DrawFramebuffer");
                m_Window->DrawFramebuffer(*m_Framebuffer);
            }
            Platform::PollInputEvents();
        }
        RGS_PROFILE_END_SESSION();
    }

}