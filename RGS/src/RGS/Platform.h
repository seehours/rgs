#pragma once

namespace RGS {
    class Platform {
    public:
        static void Init();
        static void Terminate();
        static void PollInputEvents();

    private:
        static void WindowsPollInputEventsImpl();
    };
}


