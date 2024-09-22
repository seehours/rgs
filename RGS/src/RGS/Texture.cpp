#include "rgspch.h"
#include "Texture.h"

#include "RGS/Config.h"

#include <stb_image.h>
#include <stb_image_resize2.h>

namespace RGS {

    Texture::Texture(const std::string& path)
        :m_Path(path)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        stbi_uc* data = nullptr;
        data = stbi_load(m_Path.c_str(), &width, &height, &channels, 0);
        ASSERT((data) && (width > 0) && (height > 0) && (channels > 0));

        m_Height = height;
        m_Width = width;
        m_Channels = channels;
        int size = height * width;
        m_Data = new Vec4[size];

        if (channels == 4)
        {
            for (int i = 0; i < size; i++)
            {
                m_Data[i].X = UChar2Float(data[i * 4]);
                m_Data[i].Y = UChar2Float(data[i * 4 + 1]);
                m_Data[i].Z = UChar2Float(data[i * 4 + 2]);
                m_Data[i].W = UChar2Float(data[i * 4 + 3]);
            }
        }
        else if (channels == 3)
        {
            for (int i = 0; i < size; i++)
            {
                m_Data[i].X = UChar2Float(data[i * 3]);
                m_Data[i].Y = UChar2Float(data[i * 3 + 1]);
                m_Data[i].Z = UChar2Float(data[i * 3 + 2]);
                m_Data[i].W = 0.0f;
            }
        }
        else if (channels == 2)
        {
            for (int i = 0; i < size; i++)
            {
                m_Data[i].X = UChar2Float(data[i * 2]);
                m_Data[i].Y = UChar2Float(data[i * 2 + 1]);
                m_Data[i].Z = 0.0f;
                m_Data[i].W = 0.0f;
            }
        }
        else if (channels == 1)
        {
            for (int i = 0; i < size; i++)
            {
                m_Data[i].X = UChar2Float(data[i]);
                m_Data[i].Y = 0.0f;
                m_Data[i].Z = 0.0f;
                m_Data[i].W = 0.0f;
            }
        }

        stbi_image_free(data);
    }
    
    Texture::Texture(const Framebuffer& framebuffer)
    {
        int width = framebuffer.GetWidth();
        int height = framebuffer.GetHeight();
        ASSERT((width > 0) && (height > 0));

        m_Channels = 4;
        m_Width = width;
        m_Height = height;
        int size = m_Height * m_Width;
        m_Data = new Vec4[size];
        for (int i = 0; i < size; i++)
        {
            Vec3 val = framebuffer.GetColor(i);
            m_Data[i] = { val, 1.0f };
        }
    }
    
    Texture::~Texture()
    {
        delete[] m_Data;
        m_Data = nullptr;
    }
    
    // TODO: 采样模式支持
    Vec4 Texture::Sample(const Vec2 texCoords) const
    {
        float u = Clamp(texCoords.X, 0.0f, 1.0f);
        float v = Clamp(texCoords.Y, 0.0f, 1.0f);

        u = u * (m_Width - 1) + 0.5f;
        v = v * (m_Height - 1) + 0.5f;

        int x = floor(u);
        int y = floor(v);
        float fracX = fmod(u, 1.0f);
        float fracY = fmod(v, 1.0f);

        int index0 = y * m_Width + x;
        int index1 = y * m_Width + (x + 1 == m_Width ? x : x + 1);
        int index2 = (y + 1 == m_Height ? y : y + 1) * m_Width + x;
        int index3 = (y + 1 == m_Height ? y : y + 1) * m_Width + (x + 1 == m_Width ? x : x + 1);

        Vec4 res{ 0.0f, 0.0f, 0.0f, 0.0f };
        res += m_Data[index0] * (1.0f - fracX) * (1.0f - fracY);
        res += m_Data[index1] * fracX * (1.0f - fracY);
        res += m_Data[index2] * (1.0f - fracX)* fracY;
        res += m_Data[index3] * fracX * fracY;
        return res;
    }
    
    Vec3 TextureSphere::Sample(const Vec3& v3) const
    {
        // https://blog.csdn.net/masilejfoaisegjiae/article/details/105804301
        Vec3 dir = Normalize(v3);
        float phi = atan2(dir.Z, dir.X);
        float theta = acos(dir.Y);
        float u = phi / (2 * PI) + 0.5f;
        float v = 1.0f - theta / PI;

        int x = u * (m_Width - 1) + 0.5f;
        int y = v * (m_Height - 1) + 0.5f;

        int index = y * m_Width + x;
        return m_Data[index];
    }

    TextureSphere::TextureSphere(const std::string& path)
        :m_Path(path)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        float* data;
        data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
        ASSERT((data) && (width > 0) && (height > 0) && (channels == 3));

        m_Height = height;
        m_Width = width;
        m_Channels = channels;
        m_PixelSize = height * width;

        // stb_image.h 自动将 HDR 值映射到一个浮点数列表：默认情况下，每个通道32位，每个颜色 3 个通道
        int size = height * width;
        m_Data = new Vec3[size];
        memcpy(m_Data, data, size * sizeof(Vec3));
        stbi_image_free(data);
    }

    TextureSphere::TextureSphere(const Framebuffer& framebuffer)
    {
        m_Width = framebuffer.GetWidth();
        m_Height = framebuffer.GetHeight();
        m_Channels = 3;
        int size = m_Height * m_Width;
        m_PixelSize = size;
        m_Data = new Vec3[size];
        memcpy(m_Data, framebuffer.GetRawColorData(), size * sizeof(Vec3));
    }
   
    TextureSphere::~TextureSphere()
    {
        delete[] m_Data;
        m_Data = nullptr;
    }

    TextureSphere* TextureSphere::LoadTextureSphere(const std::string& path)
    {
        int width, height, channels;
        float* data;
        stbi_set_flip_vertically_on_load(true);
        data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);

        if (data == nullptr || width <= 0 || height <= 0 || channels != 3)
        {
            std::cout << "加载失败" << std::endl;
            return nullptr;
        }

        TextureSphere* res = new TextureSphere();

        res->m_Height = height;
        res->m_Width = width;
        res->m_Channels = channels;
        res->m_PixelSize = height * width;

        int size = height * width;
        res->m_Data = new Vec3[size];
        memcpy(res->m_Data, data, size * sizeof(Vec3));
        stbi_image_free(data);
        return res;
    }

    LodTextureSphere::LodTextureSphere(std::vector<std::string> paths)
    {
        ASSERT(paths.size() == 5);

        stbi_set_flip_vertically_on_load(true);
        int width, height, channels, size;
        float* data;
        for (int i = 0; i < 5; i++)
        {
            data = stbi_loadf(paths[i].c_str(), &width, &height, &channels, 0);
            ASSERT((data) && (width > 0) && (height > 0) && (channels == 3));
            size = height * width;

            m_Data[i].Height = height;
            m_Data[i].Width = width;
            m_Data[i].Channels = channels;
            m_Data[i].PixelSize = size;
            // stb_image.h 自动将 HDR 值映射到一个浮点数列表：默认情况下，每个通道32位，每个颜色 3 个通道
            m_Data[i].ColorData = new Vec3[size];
            memcpy(m_Data[i].ColorData, data, size * sizeof(Vec3));

            stbi_image_free(data);
        }
    }

    LodTextureSphere::LodTextureSphere(std::string path)
    {
        stbi_set_flip_vertically_on_load(true);
        int in_width, in_height, in_channels;
        float* in_data;
       
        in_data = stbi_loadf(path.c_str(), &in_width, &in_height, &in_channels, 0);
        ASSERT(((in_data) && (in_width >= 32 * 2) && (in_height >= 32) && (in_channels == 3)), "非法文件");

        int width = Config::LodTextureSphereMaxWidth;
        int height = width / 2;

        for (int i = 0; i < 5; ++i)
        {
            // stb_image.h 自动将 HDR 值映射到一个浮点数列表：默认情况下，每个通道32位，每个颜色 3 个通道
            float* data = stbir_resize_float_linear(in_data, in_width, in_height, 0,
                                                    nullptr, width, height, 0, stbir_pixel_layout::STBIR_RGB);
            int size = width * height;
            m_Data[i].ColorData = new Vec3[size];
            m_Data[i].Height = height;
            m_Data[i].Width = width;
            m_Data[i].Channels = 3;
            m_Data[i].PixelSize = size;
            memcpy(m_Data[i].ColorData, data, size * sizeof(Vec3));
            stbi_image_free(data);

            width /= 2;
            height /= 2;
        }
 
        stbi_image_free(in_data);
    }

    LodTextureSphere::~LodTextureSphere()
    {
        for (auto data : m_Data)
        {
            delete data.ColorData;
        }
    }

    LodTextureSphere* LodTextureSphere::LoadLodTextureSphere(const std::string& path, LoadType loadType)
    {
        if (loadType == LoadType::SingleFile)
        {
            ASSERT(false);
        }
        else if (loadType == LoadType::Directory)
        {
            stbi_set_flip_vertically_on_load(true);
            int width, height, channels, size;
            float* data;
            LodTextureSphere* res = new LodTextureSphere();
            res->m_Path = path;
            for (int i = 0; i < 5; i++)
            {
                std::string loadPath{path};
                loadPath.append("\\");
                loadPath.append(std::to_string(i));
                loadPath.append(".hdr");

                data = stbi_loadf(loadPath.c_str(), &width, &height, &channels, 0);

                ASSERT((data) && (width > 0) && (height > 0) && (channels == 3));
                if (!((data) && (width > 0) && (height > 0) && (channels == 3)))
                {
                    std::cout << "加载失败" << std::endl;
                    stbi_image_free(data);
                    delete res;
                    return nullptr;
                }
                size = height * width;
                res->m_Data[i].Height = height;
                res->m_Data[i].Width = width;
                res->m_Data[i].Channels = channels;
                res->m_Data[i].PixelSize = size;
                res->m_Data[i].ColorData = new Vec3[size];
                memcpy(res->m_Data[i].ColorData, data, size * sizeof(Vec3));
                stbi_image_free(data);
            }
            return res;
        }
        ASSERT(false);
        return nullptr;
    }


    // TODO: 似乎有xy偏移
    Vec3 LodTextureSphere::Sample(const Vec3& v3, float lod) const
    {
        ASSERT((lod >= 0) && (lod <= (float)4));
        int number = floor(lod);
        float frac = fmod(lod, 1.0f);

        Vec3 dir = Normalize(v3);
        float phi = atan2(dir.Z, dir.X);
        float theta = acos(dir.Y);
        float u = phi / (2.0f * PI) + 0.5f;
        float v = 1.0f - theta / PI;

#if 1 // 双向过滤
        if (number == 4)
        {
            u = u * (m_Data[4].Width - 1) + 0.5f;
            v = v * (m_Data[4].Height - 1) + 0.5f;
            int x = floor(u);
            int y = floor(v);
            float fracX = fmod(u, 1.0f);
            float fracY = fmod(v, 1.0f);

            Vec3 res{ 0.0f, 0.0f, 0.0f };
            res += GetColor(x, y, 4) * (1.0f - fracX) * (1.0f - fracY);
            res += GetColor(x + 1, y, 4) * fracX * (1.0f - fracY);
            res += GetColor(x, y + 1, 4) * (1.0f - fracX) * fracY;
            res += GetColor(x + 1, y + 1, 4) * fracX * fracY;
            return res;
        }
        else
        {
            float u0 = u * (m_Data[number].Width - 1) + 0.5f;
            float v0 = v * (m_Data[number].Height - 1) + 0.5f;
            int x0 = floor(u0);
            int y0 = floor(v0);
            float fracX0 = fmod(u0, 1.0f);
            float fracY0 = fmod(v0, 1.0f);

            Vec3 c0{ 0.0f, 0.0f, 0.0f };
            c0 += GetColor(x0, y0, number) * (1.0f - fracX0) * (1.0f - fracY0);
            c0 += GetColor(x0 + 1, y0, number) * fracX0 * (1.0f - fracY0);
            c0 += GetColor(x0, y0 + 1, number) * (1.0f - fracX0) * fracY0;
            c0 += GetColor(x0 + 1, y0 + 1, number) * fracX0 * fracY0;

            float u1 = u * (m_Data[number + 1].Width - 1) + 0.5f;
            float v1 = v * (m_Data[number + 1].Height - 1) + 0.5f;
            int x1 = floor(u1);
            int y1 = floor(v1);
            float fracX1 = fmod(u1, 1.0f);
            float fracY1 = fmod(v1, 1.0f);

            Vec3 c1{ 0.0f, 0.0f, 0.0f };
            c1 += GetColor(x1, y1, number + 1) * (1.0f - fracX1) * (1.0f - fracY1);
            c1 += GetColor(x1 + 1, y1, number + 1) * fracX1 * (1.0f - fracY1);
            c1 += GetColor(x1, y1 + 1, number + 1) * (1.0f - fracX1) * fracY1;
            c1 += GetColor(x1 + 1, y1 + 1, number + 1) * fracX1 * fracY1;

            Vec3 color = Lerp(c0, c1, frac);
            return color;
        }
#else
        if (number == 4)
        {
            u = u * (m_Data[4].Width - 1) + 0.5f;
            v = v * (m_Data[4].Height - 1) + 0.5f;

            int x = floor(u);
            int y = floor(v);

            int index = (int)y * m_Data[4].Width + (int)x;
            return m_Data[4].ColorData[index];
        }
        else
        {
            int x0 = u * (m_Data[number].Width - 1) + 0.5f;
            int y0 = v * (m_Data[number].Height - 1) + 0.5f;
            int index0 = y0 * m_Data[number].Width + x0;

            int x1 = u * (m_Data[number + 1].Width - 1) + 0.5f;
            int y1 = v * (m_Data[number + 1].Height - 1) + 0.5f;
            int index1 = y1 * m_Data[number + 1].Width + x1;

            Vec3 c0 = m_Data[number].ColorData[index0];
            Vec3 c1 = m_Data[number + 1].ColorData[index1];
            Vec3 color = Lerp(c0, c1, frac);
            return color;
        }
#endif 

        return { 0.0f, 0.0f, 0.0f };
    }

}



