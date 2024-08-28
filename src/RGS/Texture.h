#pragma once
#include "RGS/Maths.h"
#include <string>

namespace RGS {

    class Texture
    {
    public:
        Texture(const std::string& path);
        ~Texture();

        Vec4 Sample(Vec2 texCoords) const;
    private:
        void Init();

    private:
        int m_Width, m_Height, m_Channels;
        std::string m_Path;
        Vec4* m_Data = nullptr;
    };

}