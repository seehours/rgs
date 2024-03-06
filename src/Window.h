#pragma once
#include <string>

namespace RGS {
    class Window
    {
    public:
        Window(const std::string title, const int width, const int height);
        virtual ~Window() {};

        virtual void Show() const = 0;

        bool Closed() const { return m_Closed; }

    public:
        static void Init();
        static void Terminate();
        static Window* Create(const std::string title, const int width, const int height);

    protected:
        std::string m_Title;
        int m_Width;
        int m_Height;
        bool m_Closed = true;
    };
}