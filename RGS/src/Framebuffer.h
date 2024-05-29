#pragma once
#include "Math.h"

#include <iostream>
namespace RGS {
    
    class Framebuffer 
    {
    public:
        void PrintDepth(const int step = 20) const
        {
            for (int y = (int)m_Height - 1 ; y >= step; y -= step)
            {
                for (int x = 0; x < (int)m_Width; x += step)
                {
                    uint32_t index = GetPixelIndex(x, y);
                    std::cout << std::fixed;
                    std::cout.precision(2);
                    std::cout << m_DepthBuffer[index] << " ";
                }
                std::cout << std::endl;
            }
        }

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

        void SetColor(const uint32_t x, const uint32_t y, const Vec3& color);
        Vec3 GetColor(const uint32_t x, const uint32_t y) const;
        void SetDepth(const uint32_t x, const uint32_t y, const float depth);
        float GetDepth(const uint32_t x, const uint32_t y) const;

        void Clear(const Vec3& color);
        void ClearDepth(float depth = 1.0f);

        Framebuffer(const uint32_t width, const uint32_t height);
        ~Framebuffer();
    private:
        uint32_t GetPixelIndex(const uint32_t x, const uint32_t y) const { return y * m_Width + x; }
    
    private:
        uint32_t m_Width = 800;
        uint32_t m_Height = 600;
        uint32_t m_PixelSize;
        float* m_DepthBuffer;
        Vec3* m_ColorBuffer;
    };

}

