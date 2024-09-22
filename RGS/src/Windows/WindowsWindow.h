#pragma once 
#include "RGS/Render/Framebuffer.h"
#include "RGS/Window.h"

#include <windows.h>

namespace RGS {

    class WindowsWindow : public Window {

    private:
        HWND m_Handle;
        HDC m_MemoryDC;
        unsigned char* m_Buffer;

        static bool s_Inited;

    public:
        WindowsWindow(const char* title, const unsigned int width, const unsigned int height);
        ~WindowsWindow();

        void DrawFramebuffer(const Framebuffer& framebuffer) override;
        void Show() override;

    public:
        void static Init();
        void static Terminate();

    private:
        void static Register();
        void static Unregister();
        static LRESULT CALLBACK WndProc(const HWND hWnd, const UINT msgID, const WPARAM wParam, const LPARAM lParam);
        static void KeyPressImpl(WindowsWindow* const window, const WPARAM wParam, const char keyState);
        static void ButtonPressImpl(WindowsWindow* const window, const char button, const char buttonState);
    };

}

