#pragma once
#include "RGS/Base/Maths.h"
#include "MSAASettings.h"

namespace RGS {
    
    class Framebuffer 
    {
    public:

        static std::unique_ptr<Framebuffer> Create(const uint32_t width, const uint32_t height, const MSAA msaa = MSAA::None);
        ~Framebuffer();

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetPixelSize() const { return m_PixelSize; }
        MSAA GetMSAA() const { return m_MSAA; }

        void SetColor(const int x, const int y, const Vec3& color);
        void SetColor(const int x, const int y, const int sampleIndex ,const Vec3& color);
        Vec3 GetColor(const uint32_t index) const;
        Vec3 GetColor(const int x, const int y) const;
        Vec3 GetColor(const int x, const int y, const int sampleIndex);
        void SetDepth(const int x, const int y, const float depth);
        void SetDepth(const int x, const int y, const int sampleIndex, const float depth);
        float GetDepth(uint32_t index) const;
        float GetDepth(const int x, const int y) const;
        float GetDepth(const int x, const int y, const int sampleIndex) const;
        const float* GetRawColorData() const { return (float*)(m_ColorBuffer); }
        std::unique_ptr<unsigned char[]> GetRGBColorData() const;

        void Clear(const Vec3& color = { 0.0f, 0.0f, 0.0f });
        void ClearDepth(float depth = 1.0f);

        void Blit(const Framebuffer& srcFramebuffer, bool copyColor = true, bool copyDepth = false);
        void Resolve();
        void ResolveParallel(const bool wait = true);

    private:

        Framebuffer(const uint32_t width, const uint32_t height, const MSAA msaa = MSAA::None);

        // Calculates the index for a pixel in the non-MSAA color/depth buffer.
        uint32_t GetPixelIndex(const uint32_t x, const uint32_t y) const { return y * m_Width + x; }

        // Calculates the index for a sample in the MSAA buffer.
        uint32_t GetRawPixelIndex(const uint32_t x, const uint32_t y, const uint32_t index) const
        {
            return (y * m_Width + x) * (int)m_MSAA + index;
        }

    private:
        uint32_t m_Width = 800;         // Framebuffer width in pixels.
        uint32_t m_Height = 600;        // Framebuffer height in pixels.
        MSAA m_MSAA = MSAA::None;       // MSAA (multi-sample anti-aliasing) factor.
        uint32_t m_PixelSize;           // Total number of pixels in the framebuffer (width * height).
        uint32_t m_RawPixelSize;        // Total number of samples in the framebuffer with MSAA (width * height * msaa2).
        float* m_DepthBuffer;           // Pointer to the depth buffer.
        Vec3* m_ColorBuffer;            // Pointer to the color buffer (non-MSAA).

        Vec3* m_RawColorBuffer;         // Pointer to the MSAA color buffer.
        float* m_RawDepthBuffer;        // Pointer to the MSAA depth buffer.
    };

}

