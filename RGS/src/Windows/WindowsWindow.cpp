#include "WindowsWindow.h"

#include "RGS/Window.h"
#include "RGS/InputCode.h"
#include "RGS/Base/Base.h"
#include "RGS/Render/Framebuffer.h"
#include "RGS/JobSystem.h"

#include <windows.h>
#include <algorithm>

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
        wc.hbrBackground = (HBRUSH)(WHITE_BRUSH);
        wc.hCursor = NULL;                              // 默认光标
        wc.hIcon = NULL;                                // 默认图标
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpfnWndProc = WindowsWindow::WndProc;       
        wc.lpszClassName = RGS_WINDOW_CLASS_NAME;
        wc.style = CS_HREDRAW | CS_VREDRAW;             // 拉伸时重绘
        wc.lpszMenuName = NULL;                         // 不要菜单
        atom = RegisterClass(&wc);                      // 注册窗口
    }

    void WindowsWindow::Unregister() 
    {
        UnregisterClass(RGS_WINDOW_CLASS_NAME, GetModuleHandle(NULL));
    }

    LRESULT CALLBACK WindowsWindow::WndProc(const HWND hWnd, 
                                            const UINT msgID, 
                                            const WPARAM wParam, 
                                            const LPARAM lParam) {
        WindowsWindow* window = (WindowsWindow*)GetProp(hWnd, RGS_WINDOW_ENTRY_NAME);
        if (window == nullptr)
            return DefWindowProc(hWnd, msgID, wParam, lParam);
        switch (msgID) {
        case WM_KILLFOCUS:
            window->Reset();
            return 0;
        case WM_DESTROY:
            window->m_Closed = true;
            return 0;
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
            {
                window->m_Minimized = true;
            }
            else 
            {
                window->m_Minimized = false;
            }
            return 0;
        case WM_KEYDOWN:
            KeyPressImpl(window, wParam, RGS_PRESS);
            return 0;
        case WM_KEYUP:
            KeyPressImpl(window, wParam, RGS_RELEASE);
            return 0;
        case WM_LBUTTONDOWN:
            ButtonPressImpl(window, RGS_BUTTON_LEFT, RGS_PRESS);
            return 0;
        case WM_LBUTTONUP:
            ButtonPressImpl(window, RGS_BUTTON_LEFT, RGS_RELEASE);
            return 0;
        case WM_RBUTTONDOWN:
            ButtonPressImpl(window, RGS_BUTTON_RIGHT, RGS_PRESS);
            return 0;
        case WM_RBUTTONUP:
            ButtonPressImpl(window, RGS_BUTTON_RIGHT, RGS_RELEASE);
            return 0;
        }
        return DefWindowProc(hWnd, msgID, wParam, lParam);
    }

    void WindowsWindow::KeyPressImpl(WindowsWindow* const window, const WPARAM wParam, const char keyState)
    {
        if (wParam >= '0' && wParam <= '9') 
        {
            window->m_Keys[wParam] = keyState;
            return;
        }

        if (wParam >= 'A' && wParam <= 'Z')
        {
            window->m_Keys[wParam] = keyState;
            return;
        }

        switch (wParam)
        {
        case VK_SPACE:
            window->m_Keys[RGS_KEY_SPACE] = keyState;
            break;
        case VK_SHIFT:
            window->m_Keys[RGS_KEY_LEFT_SHIFT] = keyState;
            break;
        default:
            break;
        }

    }

    void WindowsWindow::ButtonPressImpl(WindowsWindow* const window, const char button, const char buttonState) 
    {
        ASSERT(((button >= 0) && (button < RGS_BUTTON_MAX_COUNT)));
        window->m_Buttons[button] = buttonState;
    }

    void WindowsWindow::Show() 
    {
        HDC windowDC = GetDC(m_Handle);
        BitBlt(windowDC, 0, 0, (int)m_Width, (int)m_Height, m_MemoryDC, 0, 0, SRCCOPY);
        ShowWindow(m_Handle, SW_SHOW);
        ReleaseDC(m_Handle, windowDC);
    }

    void WindowsWindow::DrawFramebuffer(const Framebuffer& framebuffer)
    {
        const uint32_t fWidth = framebuffer.GetWidth();
        const uint32_t fHeight = framebuffer.GetHeight();
        const uint32_t width = (std::min)((uint32_t)m_Width, fWidth);
        const uint32_t height = (std::min)((uint32_t)m_Height, fHeight);

        uint32_t jobCount = width * height;
        constexpr uint32_t groupSize = 2048u;
        JobSystem::Dispatch(jobCount, groupSize, [=, &framebuffer](JobSystem::JobDispatchArgs args)
        {
            int x = args.JobIndex % width;
            int y = args.JobIndex / width;

            constexpr uint32_t channelCount = 3;
            constexpr uint32_t rChannel = 2;
            constexpr uint32_t gChannel = 1;
            constexpr uint32_t bChannel = 0;

            Vec3 color = framebuffer.GetColor(x, y);
            const uint32_t pixStart = (x + (height - 1 - y) * m_Width) * channelCount;
            const uint32_t rIndex = pixStart + rChannel;
            const uint32_t gIndex = pixStart + gChannel;
            const uint32_t bIndex = pixStart + bChannel;

            // 翻转RGB显示
            m_Buffer[rIndex] = Float2UChar(color.X);
            m_Buffer[gIndex] = Float2UChar(color.Y);
            m_Buffer[bIndex] = Float2UChar(color.Z);
         });

        JobSystem::Wait();
        Show();
    }

    WindowsWindow::WindowsWindow(const char* title, const uint32_t width, const uint32_t height)
        :Window(title, width, height) 
    {
        ASSERT((s_Inited), "未初始化，尝试 RGS::Platform::Init()");

        DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.bottom = (long)height;
        rect.right = (long)width;
        AdjustWindowRect(&rect, style, false);
        m_Handle = CreateWindow(RGS_WINDOW_CLASS_NAME, m_Title, style,
                                400, 100, rect.right - rect.left, rect.bottom - rect.top,
                                NULL, NULL, GetModuleHandle(NULL), NULL);
        ASSERT(m_Handle != nullptr);
        m_Closed = false;
        m_Minimized = false;
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
        newBitmap = CreateDIBSection(m_MemoryDC, (BITMAPINFO*)&biHeader, DIB_RGB_COLORS, (void**)&m_Buffer, nullptr, 0);
        ASSERT(newBitmap != nullptr);
        constexpr uint32_t channelCount = 3;
        size_t size = m_Width * m_Height * channelCount * sizeof(unsigned char);
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

