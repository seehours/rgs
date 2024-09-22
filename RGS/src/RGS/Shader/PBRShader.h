#pragma once
#include "ShaderBase.h"

namespace RGS {

    struct PBRVertex : public VertexBase
    {
        Vec3 Normal = { 1.0f, 0.0f, 0.0f };
    };

    struct PBRVaryings : public VaryingsBase
    {
        Vec3 WorldPos;
        Vec3 WorldNormal;
    };

    struct PBRUniforms : public UniformsBase
    {
        Mat4 Model;
        Mat4 ModelNormalToWorld;

        Vec3 Albedo;
        float Metallic;
        float Roughness;
        float Ao;

        Vec3 LightPositions[4];
        Vec3 LightColors[4];

        Vec3 CamPos;
    };

    void PBRVertexShader(PBRVaryings& varyings, const PBRVertex& vertex, const PBRUniforms& uniforms);

    Vec4 PBRFragmentShader(bool& discard, const PBRVaryings& varyings, const PBRUniforms& uniforms);
}

