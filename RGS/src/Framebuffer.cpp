#pragma once
#include "Base.h"
#include "Framebuffer.h"

namespace RGS {
   
    Framebuffer::Framebuffer(const uint32_t width, const uint32_t height)
        :m_Width(width), m_Height(height)
    {
        m_PixelSize = m_Width * m_Height;
        m_ColorBuffer = new Vec3[m_PixelSize]();
        m_DepthBuffer = new float[m_PixelSize]();
        Clear({ 0.0f, 0.0f, 0.0f });
        ClearDepth(1.0f);
    }
    Framebuffer::~Framebuffer()
    {
        delete[] m_ColorBuffer;
        delete[] m_DepthBuffer;
        m_ColorBuffer = nullptr;
        m_DepthBuffer = nullptr;
    }

    void Framebuffer::SetColor(const uint32_t x, const uint32_t y, const Vec3& color)
    {
        uint32_t index = GetPixelIndex(x, y);
        if (index < m_PixelSize)
        {
            m_ColorBuffer[index] = color;
        }
        else
        {
            ASSERT(false);
        }
    }

    Vec3 Framebuffer::GetColor(const uint32_t x, const uint32_t y) const
    {
        uint32_t index = GetPixelIndex(x, y);
        if (index < m_PixelSize)
        {
            return m_ColorBuffer[index];
        }
        else 
        {
            ASSERT(false);
            return { 0.0f, 0.0f ,0.0f };
        }
    }


    void Framebuffer::SetDepth(const uint32_t x, const uint32_t y, const float depth)
    {
        uint32_t index = GetPixelIndex(x, y);
        if (index < m_PixelSize)
        {
            m_DepthBuffer[index] = depth;
        }
        else
        {
            ASSERT(false);
        }
    }

    float Framebuffer::GetDepth(const uint32_t x, const uint32_t y) const
    {
        uint32_t index = GetPixelIndex(x, y);
        if (index < m_PixelSize)
        {
            float depth = m_DepthBuffer[index];
            return depth;
        }
        else
        {
            ASSERT(false);
            return 0.0f;
        }
    }


    void Framebuffer::Clear(const Vec3& color)
    {
        for (uint32_t i = 0; i < m_PixelSize; i++)
        {
            m_ColorBuffer[i] = color;
        }
    }

    void Framebuffer::ClearDepth(float depth)
    {
        for (uint32_t i = 0; i < m_PixelSize; i++)
        {
            m_DepthBuffer[i] = depth;
        }
    }
}

