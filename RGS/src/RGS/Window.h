#pragma once 
#include "RGS/Render/Framebuffer.h"
#include "RGS/InputCode.h"

namespace RGS {

    class Window 
    {
    public:
        Window(const char* title, const int width, const int height);
        virtual ~Window() {};

        virtual void DrawFramebuffer(const Framebuffer& framebuffer) = 0;
        virtual void Show() = 0;

        bool Closed() const { return m_Closed; } 
        bool Minimized() const { return m_Minimized; }
        char GetKey(const int index) const { return m_Keys[index]; }
        char GetButton(const int index) const { return m_Buttons[index]; }

    protected:
        void Reset();

    public:
        static void Init();
        static void Terminate();
        static Window* Create(const char* title, const int width, const int height);

    protected:
        const char* m_Title;
        int m_Width;
        int m_Height;
        bool m_Closed;
        bool m_Minimized;

        char m_Keys[RGS_KEY_MAX_COUNT];
        char m_Buttons[RGS_BUTTON_MAX_COUNT];
    };

}

