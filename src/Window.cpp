#include "Window.h"

#include "Base.h"
#include "WindowsWindow.h"

namespace RGS {

    Window::Window(std::string title, const int width, const int height)
        : m_Title(title), m_Width(width), m_Height(height) 
    {
        ASSERT((m_Width > 0) && (m_Height > 0));
        memset(m_Keys, RGS_RELEASE, RGS_KEY_MAX_COUNT);
    }

    void Window::Init()
    {
        WindowsWindow::Init();
    }

    void Window::Terminate()
    {
        WindowsWindow::Terminate();
    }

    Window* Window::Create(const std::string title, const int width, const int height)
    {
        return new WindowsWindow(title, width, height);
    }

    void Window::PollInputEvents()
    {
        WindowsWindow::PollInputEvents();
    }


}