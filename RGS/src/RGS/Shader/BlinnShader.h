#pragma once
#include "ShaderBase.h"

namespace RGS {

    struct BlinnVertex : public VertexBase
    {
        Vec3 ModelNormal = {1.0f, 0.0f, 0.0f};
        Vec2 TexCoord = { 0.0f, 0.0f };
    };

    struct BlinnVaryings : public VaryingsBase
    {
        Vec3 WorldPos;
        Vec3 WorldNormal;
        Vec2 TexCoord;
    };

    struct BlinnUniforms : public UniformsBase
    {
        Mat4 Model;
        Mat4 ModelNormalToWorld;
        Vec3 LightPos;
        Vec3 CameraPos;

        Texture* Diffuse = nullptr;
        Texture* Specular = nullptr;
    };

    void BlinnVertexShader(BlinnVaryings& varyings, const BlinnVertex& vertex, const BlinnUniforms& uniforms);

    Vec4 BlinnFragmentShader(bool& discard, const BlinnVaryings& varyings, const BlinnUniforms& uniforms);

}