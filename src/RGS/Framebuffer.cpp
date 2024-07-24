#pragma once
#include "Base.h"
#include "Framebuffer.h"

namespace RGS {

    Framebuffer::Framebuffer(const int width, const int height)
        :m_Width(width), m_Height(height)
    {
        ASSERT((width > 0) && (height > 0));
        m_PixelSize = m_Width * m_Height;
        m_ColorBuffer = new Vec3[m_PixelSize]();
        m_DepthBuffer = new float[m_PixelSize]();
        Clear();
        ClearDepth();
    }

    Framebuffer::~Framebuffer()
    {
        delete[] m_ColorBuffer;
        delete[] m_DepthBuffer;
        m_ColorBuffer = nullptr;
        m_DepthBuffer = nullptr;
    }

    void Framebuffer::SetColor(const int x, const int y, const Vec3& color)
    {
        if ((x < 0) || (x >= m_Width) || (y < 0) || (y >= m_Height))
        {
            ASSERT(false);
            return;
        }
        else
        {
            int index = GetPixelIndex(x, y);
            m_ColorBuffer[index] = color;
        }
    }

    Vec3 Framebuffer::GetColor(const int x, const int y) const
    {
        if ((x < 0) || (x >= m_Width) || (y < 0) || (y >= m_Height))
        {
            ASSERT(false);
            return {0.0f, 0.0f, 0.0f};
        }
        else
        {
            int index = GetPixelIndex(x, y);
            return m_ColorBuffer[index];
        }
    }

    void Framebuffer::SetDepth(const int x, const int y, const float depth)
    {

        if ((x < 0) || (x >= m_Width) || (y < 0) || (y >= m_Height))
        {
            ASSERT(false);
            return;
        }
        else
        {
            int index = GetPixelIndex(x, y);
            m_DepthBuffer[index] = depth;
        }
    }

    float Framebuffer::GetDepth(const int x, const int y) const
    {
        if ((x < 0) || (x >= m_Width) || (y < 0) || (y >= m_Height))
        {
            ASSERT(false);
            return 0.0f;
        }
        else
        {
            int index = GetPixelIndex(x, y);
            float depth = m_DepthBuffer[index];
        }
    }

    void Framebuffer::Clear(const Vec3& color)
    {
        for (int i = 0; i < m_PixelSize; i++)
        {
            m_ColorBuffer[i] = color;
        }
    }

    void Framebuffer::ClearDepth(float depth)
    {
        for (int i = 0; i < m_PixelSize; i++)
        {
            m_DepthBuffer[i] = depth;
        }
    }
}

