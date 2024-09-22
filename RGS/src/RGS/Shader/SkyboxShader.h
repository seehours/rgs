#pragma once
#include "ShaderBase.h"

namespace RGS {

    using SkyboxVertex = VertexBase3D;

    struct SkyboxVaryings : public VaryingsBase
    {
        Vec3 TexPos = { 0.0f ,0.0f, 0.0f };
    };

    struct SkyboxUniforms : public UniformsBase
    {
        TextureSphere* SkyboxTex;
        LodTextureSphere* LodSkyboxTex = nullptr;
        float Lod = 0.0f;
    };

    void SkyboxVertexShader(SkyboxVaryings& varyings, const SkyboxVertex& vertex, const SkyboxUniforms& uniforms);

    Vec4 SkyboxFragmentShader(bool& discard, const SkyboxVaryings& varyings, const SkyboxUniforms& uniforms);
}