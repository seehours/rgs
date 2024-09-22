#include "rgspch.h"
#include "Framebuffer.h"
#include "RGS/JobSystem.h"

namespace RGS {

    std::unique_ptr<Framebuffer> Framebuffer::Create(const uint32_t width, const uint32_t height, const MSAA msaa)
    {
        std::unique_ptr<Framebuffer> framebuffer(new Framebuffer(width, height, msaa));
        return framebuffer;
    }

    Framebuffer::Framebuffer(const uint32_t width, const uint32_t height, const MSAA msaa)
        :m_Width(width), m_Height(height), m_MSAA(msaa), 
        m_PixelSize (width * height), m_RawPixelSize(width * height * (int)msaa)
    {
        m_ColorBuffer = new Vec3[m_PixelSize]();
        m_DepthBuffer = new float[m_PixelSize]();

        m_RawColorBuffer = new Vec3[m_RawPixelSize]();
        m_RawDepthBuffer = new float[m_RawPixelSize]();

        Clear();
        ClearDepth();
    }

    Framebuffer::~Framebuffer()
    {
        delete[] m_ColorBuffer;
        delete[] m_DepthBuffer;
        delete[] m_RawColorBuffer;
        delete[] m_RawDepthBuffer;
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
            uint32_t index = GetPixelIndex((uint32_t)x, (uint32_t)y);
            m_ColorBuffer[index] = color;
        }
    }

    void Framebuffer::SetColor(const int x, const int y, const int sampleIndex, const Vec3& color)
    {
        if ((x < 0) || (x >= m_Width) || (y < 0) || (y >= m_Height) || (sampleIndex < 0) || (sampleIndex >= (int)m_MSAA))
        {
            ASSERT(false);
            return;
        }
        else
        {
            uint32_t index = GetRawPixelIndex((uint32_t)x, (uint32_t)y, (uint32_t)sampleIndex);
            m_RawColorBuffer[index] = color;
        }
    }

    Vec3 Framebuffer::GetColor(const uint32_t index) const
    {
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

    Vec3 Framebuffer::GetColor(const int x, const int y) const
    {
        if ((x < 0) || (x >= m_Width) || (y < 0) || (y >= m_Height))
        {
            ASSERT(false);
            return { 0.0f, 0.0f ,0.0f };
        }
        else
        {
            uint32_t index = GetPixelIndex((uint32_t)x, (uint32_t)y);
            return m_ColorBuffer[index];
        }
    }

    Vec3 Framebuffer::GetColor(const int x, const int y, const int sampleIndex)
    {
        if ((x < 0) || (x >= m_Width) || (y < 0) || (y >= m_Height) || (sampleIndex < 0) || (sampleIndex >= (int)m_MSAA))
        {
            ASSERT(false);
            return { 0.0f, 0.0f ,0.0f };
        }
        else
        {
            uint32_t index = GetRawPixelIndex((uint32_t)x, (uint32_t)y, (uint32_t)sampleIndex);
            return m_RawColorBuffer[index];
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
            uint32_t index = GetPixelIndex((uint32_t)x, (uint32_t)y);
            m_DepthBuffer[index] = depth;
        }
    }

    void Framebuffer::SetDepth(const int x, const int y, const int sampleIndex, const float depth)
    {
        if ((x < 0) || (x >= m_Width) || (y < 0) || (y >= m_Height) || (sampleIndex < 0) || (sampleIndex >= (int)m_MSAA))
        {
            ASSERT(false);
            return;
        }
        else
        {
            uint32_t index = GetRawPixelIndex((uint32_t)x, (uint32_t)y, (uint32_t)sampleIndex);
            m_RawDepthBuffer[index] = depth;
        }
    }

    float Framebuffer::GetDepth(uint32_t index) const
    {
        if (index < m_PixelSize)
        {
            return m_DepthBuffer[index];
        }
        else
        {
            ASSERT(false);
            return 0.0f;
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
            uint32_t index = GetPixelIndex((uint32_t)x, (uint32_t)y);
            return m_DepthBuffer[index];
        }
    }

    float Framebuffer::GetDepth(const int x, const int y, const int sampleIndex) const
    {
        if ((x < 0) || (x >= m_Width) || (y < 0) || (y >= m_Height) || (sampleIndex < 0) || (sampleIndex >= (int)m_MSAA))
        {
            ASSERT(false);
            return 0.0f;
        }
        else
        {
            uint32_t index = GetRawPixelIndex((uint32_t)x, (uint32_t)y, (uint32_t)sampleIndex);
            return m_RawDepthBuffer[index];
        }
    }

    void Framebuffer::Clear(const Vec3& color)
    {
        std::fill(m_ColorBuffer, m_ColorBuffer + m_PixelSize, color);
        std::fill(m_RawColorBuffer, m_RawColorBuffer + m_RawPixelSize, color);
    }

    void Framebuffer::ClearDepth(float depth)
    {
        std::fill(m_DepthBuffer, m_DepthBuffer + m_PixelSize, depth);
        std::fill(m_RawDepthBuffer, m_RawDepthBuffer + m_RawPixelSize, depth);
    }

    std::unique_ptr<unsigned char[]> Framebuffer::GetRGBColorData() const
    {
        constexpr int channel = 3;
        int size = m_Width * m_Height;
        std::unique_ptr<unsigned char[]> ptr(new unsigned char[size * channel]);
        for (int i = 0; i < size; ++i)
        {
            ptr[i * channel + 0] = Float2UChar(m_ColorBuffer[i].X);
            ptr[i * channel + 1] = Float2UChar(m_ColorBuffer[i].Y);
            ptr[i * channel + 2] = Float2UChar(m_ColorBuffer[i].Z);
        }
        return ptr;
    }

    void Framebuffer::Resolve() 
    {
        for (int y = 0; y < m_Height; ++y)
        {
            for (int x = 0; x < m_Width; ++x) 
            {
                Vec3 finalColor = { 0.0f, 0.0f, 0.0f };
                for (int sample = 0; sample < (int)m_MSAA; ++sample)
                {
                    finalColor += m_RawColorBuffer[GetRawPixelIndex(x, y, sample)];
                }
                finalColor /= (float)m_MSAA; // 求平均值
                m_ColorBuffer[GetPixelIndex(x, y)] = finalColor; // 存储最终颜色值
            }
        }
    }

    void Framebuffer::ResolveParallel(const bool wait)
    {
        RGS_PROFILE_FUNCTION();
        uint32_t jobCount = m_PixelSize;
        constexpr uint32_t groupSize = 1024u;
        JobSystem::Dispatch(jobCount, groupSize, [this](JobSystem::JobDispatchArgs args)
        {
            uint32_t idx = args.JobIndex;

            Vec3 finalColor = { 0.0f, 0.0f, 0.0f };

            for (uint32_t sample = 0; sample < (uint32_t)m_MSAA; ++sample)
            {
                uint32_t sampleIdx = idx * (uint32_t)m_MSAA + sample;
                finalColor += m_RawColorBuffer[sampleIdx];
            }

            finalColor /= (float)m_MSAA;
            m_ColorBuffer[idx] = finalColor;
        });

        if (wait)
            JobSystem::Wait();
    }

    void Framebuffer::Blit(const Framebuffer& srcFramebuffer, bool copyColor, bool copyDepth)
    {
        RGS_PROFILE_FUNCTION();
        if (srcFramebuffer.GetWidth() == m_Width)
        {
            uint32_t minSize = std::min(m_PixelSize, srcFramebuffer.GetPixelSize());

            if (copyColor)
            {
                for (int i = 0; i < minSize; ++i)
                {
                    m_ColorBuffer[i] = srcFramebuffer.GetColor(i);
                }
            }

            if (copyDepth)
            {
                for (int i = 0; i < minSize; ++i)
                {
                    m_DepthBuffer[i] = srcFramebuffer.GetDepth(i);
                }
            }
            return;
        }

        // 处理大小不同的情况
        uint32_t minWidth = std::min(m_Width, srcFramebuffer.GetWidth());
        uint32_t minHeight = std::min(m_Height, srcFramebuffer.GetHeight());

        if (copyColor)
        {
            for (uint32_t y = 0; y < minHeight; ++y)
            {
                for (uint32_t x = 0; x < minWidth; ++x)
                {
                    uint32_t index = GetPixelIndex(x, y);
                    m_ColorBuffer[index] = srcFramebuffer.GetColor(x, y);
                }
            }
        }

        if (copyDepth)
        {
            for (uint32_t y = 0; y < minHeight; ++y)
            {
                for (uint32_t x = 0; x < minWidth; ++x)
                {
                    uint32_t index = GetPixelIndex(x, y);
                    m_DepthBuffer[index] = srcFramebuffer.GetDepth(x, y);
                }
            }
        }
    }

}

