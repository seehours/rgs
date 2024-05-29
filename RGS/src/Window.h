#pragma once 
#include "Framebuffer.h"
#include "InputCode.h"

namespace RGS {
    class Window 
    {
    public:

        Window(const char* title, const uint32_t width, const uint32_t height);

        virtual ~Window() {};

        virtual void DrawFramebuffer(const Framebuffer& framebuffer) = 0;
        virtual void Show() = 0;

        bool Closed() const { return m_Closed; } 
        char GetKey(const uint32_t index) const { return m_Keys[index]; }
        char GetButton(const uint32_t index) const { return m_Buttons[index]; }

    public:
        static void Init();
        static void Terminate();
        static Window* Create(const char* title, const uint32_t width, const uint32_t height);

    protected:
        const char* m_Title;
        uint32_t m_Width;
        uint32_t m_Height;
        bool m_Closed;

        char m_Keys[RGS_KEY_MAX_COUNT];
        char m_Buttons[RGS_BUTTON_MAX_COUNT];
    };
}

