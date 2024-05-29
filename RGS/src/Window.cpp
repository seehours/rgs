#include "Window.h"
#include "WindowsWindow.h"

namespace RGS {

    Window::Window(const char* title, const uint32_t width, const uint32_t height)
        : m_Title(title), m_Width(width), m_Height(height), m_Closed(true)
    {
        memset(m_Keys, RGS_KEY_MAX_COUNT, RGS_RELEASE);
        memset(m_Buttons, RGS_BUTTON_MAX_COUNT, RGS_RELEASE);
    }

    void Window::Init() 
    {
        WindowsWindow::Init();
    }
    
    void Window::Terminate() 
    {
        WindowsWindow::Terminate();
    }

    Window* Window::Create(const char* title, const uint32_t width, const uint32_t height) 
    {
        return new WindowsWindow(title, width, height);
    }

}