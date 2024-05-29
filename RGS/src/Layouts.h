#pragma once
#include "Math.h"
#include "Texture.h"

#include <string>
#include <type_traits>

// Vertex
namespace RGS {

    struct VertexBase
    {
        Vec4 ModelPos = { 0, 0, 0, 0 };

        VertexBase() = default;
        VertexBase(const float x, const float y, const float z, const float w)
            : ModelPos( x, y, z, w ) {}

        operator const std::string() const { return "ModelPos: " + (std::string)ModelPos; }

    };

    struct SimpleVertex : public VertexBase
    {
        Vec4 Color = {0, 0, 0, 0};
        SimpleVertex() = default;
        SimpleVertex(const float x, const float y, const float z, const float w)
            :VertexBase(x, y, z, w) {}
        SimpleVertex(const float x, const float y, const float z, const float w, const Vec4 color)
            :VertexBase(x, y, z, w), Color(color) {}
    };

    struct BlinnVertex : public VertexBase
    {
        Vec3 ModelNormal;
        Vec2 TexCoord = {0.0f, 0.0f};
    };
}

// Varyings
namespace RGS {

    struct VaryingsBase
    {
        Vec4 ClipPos = {0, 0, 0, 0};
        Vec4 NdcPos  = {0, 0, 0, 0};
        Vec4 FragPos = {0, 0, 0, 0};
    };

    struct SimpleVaryings : public VaryingsBase
    {
        Vec4 Color = { 0, 0, 0, 1};
    };

    struct BlinnVaryings : public VaryingsBase
    {
        Vec3 WorldPos;
        Vec3 WorldNormal;
        Vec2 TexCoord;
    };
}

// Uniform
namespace RGS {

    struct UniformsBase
    {
        Mat4 MVP;
        operator const std::string() const { return (std::string)MVP; }
    };

    struct SimpleUniforms : public UniformsBase  { };

    struct BlinnUniforms : public UniformsBase
    {
        Mat4 Model;
        Mat4 ModelNormalToWorld;
        Vec3 LightPos;
        Vec3 CameraPos;

        Texture *Diffuse = nullptr;
        Texture *Specular = nullptr;
    };

}

// Triangle
namespace RGS {
    template<typename T>
    struct Triangle
    {
        static_assert(std::is_base_of_v<VertexBase, T>, "Triangle 的模板参数必须继承自 RGS::VertexBase");
        using vertex_t = T;

        vertex_t Vertex[3];

        vertex_t& operator[](size_t i) { return Vertex[i]; }
        const vertex_t& operator[](size_t i) const { return Vertex[i]; }

        Triangle() = default;
    };
}


