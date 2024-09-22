#pragma once 
#include "RGS/Window.h"
#include "RGS/Render/Framebuffer.h"
#include "RGS/Layer/Layer.h"
#include "ImGui/ImGuiWindow.h"

#include <chrono>

namespace RGS {

    class Application
    {
    public:
        Application(const char* name, const uint32_t width, const uint32_t height);
        ~Application();

        void Run();
        Framebuffer& GetFramebuffer() { return *m_Framebuffer; }
        Window& GetWindow() { return *m_Window; }

        static Application& Instance() { return *s_Instance; }

    private:
        void Init();
        void Terminate();

        float GetDeltaTime();

    private:
        const char* m_Name;
        uint32_t m_Width;
        uint32_t m_Height;
        std::chrono::steady_clock::time_point m_LastFrameTime;

        std::unique_ptr<Framebuffer> m_Framebuffer;
        Window* m_Window;
        ImGuiWindow* m_ImGuiWindow;

        LayerStack m_LayerStack;

        static Application* s_Instance;

    };
}