#include "Base.h"
#include "Window.h"
#include "WindowsWindow.h"

#include <windows.h>

#define RGS_WINDOW_ENTRY_NAME  "Entry"
#define RGS_WINDOW_CLASS_NAME  "Class"

namespace RGS {

    bool WindowsWindow::s_Inited = false;

    void WindowsWindow::Init()
    {
        ASSERT(!s_Inited);
        Register();
        s_Inited = true;
    }

    void WindowsWindow::Terminate()
    {
        ASSERT(s_Inited);
        Unregister();
        s_Inited = false;
    }

    void WindowsWindow::Register()
    {
        ATOM atom;
        WNDCLASS wc = { 0 };
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hbrBackground = (HBRUSH)(WHITE_BRUSH);       // 背景色
        wc.hCursor = NULL;                              // 默认光标
        wc.hIcon = NULL;                                // 默认图标
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpfnWndProc = WindowsWindow::WndProc;        // 窗口处理函数
        wc.lpszClassName = RGS_WINDOW_CLASS_NAME;
        wc.style = CS_HREDRAW | CS_VREDRAW;             // 拉伸时重绘
        wc.lpszMenuName = NULL;                         // 不要菜单
        atom = RegisterClass(&wc);                      // 注册窗口
    }

    void WindowsWindow::Unregister()
    {
        UnregisterClass(RGS_WINDOW_CLASS_NAME, GetModuleHandle(NULL));
    }

    LRESULT CALLBACK WindowsWindow::WndProc(const HWND hWnd, const UINT msgID, const WPARAM wParam, const LPARAM lParam)
    {
        WindowsWindow* window = (WindowsWindow*)GetProp(hWnd, RGS_WINDOW_ENTRY_NAME);
        if (window == nullptr)
            return DefWindowProc(hWnd, msgID, wParam, lParam);

        switch (msgID)
        {
        case WM_DESTROY:
            window->m_Closed = true;
            return 0;
        }
        return DefWindowProc(hWnd, msgID, wParam, lParam);
    }

    void WindowsWindow::Show() const
    {
        HDC windowDC = GetDC(m_Handle);
        BitBlt(windowDC, 0, 0, m_Width, m_Height, m_MemoryDC, 0, 0, SRCCOPY);
        ShowWindow(m_Handle, SW_SHOW);
        ReleaseDC(m_Handle, windowDC);
    }

    WindowsWindow::WindowsWindow(const std::string title, const int width, const int height)
        :Window(title, width, height)
    {
        ASSERT((s_Inited), "未初始化");

        DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.bottom = (long)height;
        rect.right = (long)width;
        AdjustWindowRect(&rect, style, false);
        m_Handle = CreateWindow(RGS_WINDOW_CLASS_NAME, m_Title.c_str(), style,
                                CW_USEDEFAULT, 0, rect.right - rect.left, rect.bottom - rect.top,
                                NULL, NULL, GetModuleHandle(NULL), NULL);
        ASSERT(m_Handle != nullptr);
        m_Closed = false;
        SetProp(m_Handle, RGS_WINDOW_ENTRY_NAME, this);

        HDC windowDC = GetDC(m_Handle);
        m_MemoryDC = CreateCompatibleDC(windowDC);

        BITMAPINFOHEADER biHeader = {};
        HBITMAP newBitmap;
        HBITMAP oldBitmap;

        biHeader.biSize = sizeof(BITMAPINFOHEADER);
        biHeader.biWidth = ((long)m_Width);
        biHeader.biHeight = -((long)m_Height);
        biHeader.biPlanes = 1;
        biHeader.biBitCount = 24;
        biHeader.biCompression = BI_RGB;

        // 分配空间
        // https://learn.microsoft.com/zh-cn/windows/win32/api/wingdi/nf-wingdi-createdibsection
        newBitmap = CreateDIBSection(m_MemoryDC, (BITMAPINFO*)&biHeader, DIB_RGB_COLORS, (void**)&m_Buffer, nullptr, 0);
        ASSERT(newBitmap != nullptr);
        constexpr int channelCount = 3;
        int size = m_Width * m_Height * channelCount * sizeof(unsigned char);
        memset(m_Buffer, 0, size);
        oldBitmap = (HBITMAP)SelectObject(m_MemoryDC, newBitmap);
        
        DeleteObject(oldBitmap);
        ReleaseDC(m_Handle, windowDC);

        Show();
    }

    WindowsWindow::~WindowsWindow()
    {
        ShowWindow(m_Handle, SW_HIDE);
        RemoveProp(m_Handle, RGS_WINDOW_ENTRY_NAME);
        DeleteDC(m_MemoryDC);
        DestroyWindow(m_Handle);
    }
}