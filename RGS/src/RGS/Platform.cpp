#include "rgspch.h"
#include "Platform.h"
#include "RGS/Window.h"
#include <windows.h>

namespace RGS {
    
    void Platform::Init() 
    {
        Window::Init();
    }

    void Platform::Terminate()
    {
        Window::Terminate();
    }

    void Platform::PollInputEvents() 
    {
        WindowsPollInputEventsImpl();
    }

    void Platform::WindowsPollInputEventsImpl()
    {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

}

