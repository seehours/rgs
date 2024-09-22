#pragma once
#include "RGS/Base/Maths.h"
#include "RGS/Texture.h"

#include <string>

namespace RGS {

    struct VertexBase
    {
        Vec4 ModelPos = { 0, 0, 0, 0 };

        VertexBase() = default;
        VertexBase(const float x, const float y, const float z, const float w)
            : ModelPos( x, y, z, w ) {}

        operator const std::string() const { return "ModelPos: " + (std::string)ModelPos; }

    };

    struct VertexBase3D : public VertexBase
    {
        Vec3 ModelNormal;
    };

    struct VaryingsBase
    {
        Vec4 ClipPos = { 0.0f, 0.0f, 0.0f, 1.0f };
        Vec4 NdcPos  = { 0.0f, 0.0f, 0.0f, 1.0f };
        Vec4 FragPos = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    struct UniformsBase
    {
        Mat4 MVP;
        operator const std::string() const { return (std::string)MVP; }
    };
}

