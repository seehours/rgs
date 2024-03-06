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

    public:
        static void Init();
        static void Terminate();

    private:
        static void Register();
        static void Unregister();

        static LRESULT CALLBACK WndProc(const HWND hWnd, const UINT msgID, const WPARAM wParam, const LPARAM lParam);

    private:
        HWND m_Handle;
        HDC m_MemoryDC;
        unsigned char* m_Buffer;

        static bool s_Inited;
    };

}