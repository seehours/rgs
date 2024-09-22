#pragma once
#include "RGS/Base/Maths.h"
#include "RGS/Render/Framebuffer.h"

#include <string>
#include <vector>

namespace RGS {

    class Texture
    {
    public:
        Texture(const std::string& path);
        Texture(const Framebuffer& framebuffer);
        ~Texture();

        Vec4 Sample(const Vec2 texCoords) const;

        int GetWidth() { return m_Width; }
        int GetHeight() { return m_Height; }

    protected:
        int m_Width, m_Height, m_Channels;
        std::string m_Path;
        Vec4* m_Data;
    };

    class TextureSphere 
    {
    public:
        TextureSphere(const std::string& path);
        TextureSphere(const Framebuffer& framebuffer);
        ~TextureSphere();

        Vec3 Sample(const Vec3& v3) const;

        int GetWidth() { return m_Width; }
        int GetHeight() { return m_Height; }

        std::string GetPath() { return m_Path; }

        static TextureSphere* LoadTextureSphere(const std::string& path);

    protected:
        TextureSphere() = default;

    protected:
        int m_Width, m_Height, m_Channels, m_PixelSize;
        std::string m_Path;
        Vec3* m_Data;
    };

    class LodTextureSphere
    {
    public:
        LodTextureSphere(std::vector<std::string> paths);
        LodTextureSphere(std::string paths);
        ~LodTextureSphere();
        Vec3 Sample(const Vec3& v3, float lod) const;

        Vec3 GetColor(int x, int y, int lod) const
        {
            const Data& data = m_Data[lod];
            int width = data.Width;
            int height = data.Height;
            x %= width;
            y %= height;
            return data.ColorData[y * width + x];
        }

        enum class LoadType
        {
            SingleFile, // �Զ����ɴμ�
            Directory,  // ��Ŀ¼�²���
        };

        static LodTextureSphere* LoadLodTextureSphere(const std::string& path, LoadType loadType);


        std::string GetPath() { return m_Path; }
    protected:

        LodTextureSphere() = default;

        std::string m_Path;

        struct Data
        {
            int Width, Height, Channels, PixelSize;
            Vec3* ColorData;
        };
        Data m_Data[5]; // 0 ��Ϊ���
    };

}