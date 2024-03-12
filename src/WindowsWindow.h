#pragma once 
#include "Window.h"

#include <string>
#include <windows.h>

namespace RGS {

    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const std::string title, const int width, const int height);
        ~WindowsWindow();

        virtual void Show() const override;
        virtual void DrawFramebuffer(const Framebuffer& framebuffer);


    public:
        static void Init();
        static void Terminate();

        static void PollInputEvents();

    private:
        static void Register();
        static void Unregister();

        static void KeyPressImpl(WindowsWindow* window, const WPARAM wParam, const char state);
        static LRESULT CALLBACK WndProc(const HWND hWnd, const UINT msgID, const WPARAM wParam, const LPARAM lParam);


    private:
        HWND m_Handle;
        HDC m_MemoryDC;
        unsigned char* m_Buffer;

        static bool s_Inited;
    };

}