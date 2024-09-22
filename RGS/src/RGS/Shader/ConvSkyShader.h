#pragma once
#include "SkyboxShader.h"

namespace RGS {

    struct ConvSkyVertex : public VertexBase
    {
    };

    struct ConvSkyVaryings : public VaryingsBase
    {
    };

    struct ConvSkyUniforms : public UniformsBase
    {
        TextureSphere* SkyboxTex;
    };

    using PrefilterVertex = ConvSkyVertex;

    using PrefilterVaryings = ConvSkyVaryings;

    struct PrefilterUniforms : public UniformsBase
    {
        LodTextureSphere* SkyboxTex;
        float Roughness = 0.5f;
    };


    void ConvSkyVertexShader(ConvSkyVaryings& varyings, const ConvSkyVertex& vertex, const ConvSkyUniforms& uniforms);

    Vec4 ConvSkyFragmentShader(bool& discard, const ConvSkyVaryings& varyings, const ConvSkyUniforms& uniforms);


    void PrefilterVertexShader(PrefilterVaryings& varyings, const PrefilterVertex& vertex, const PrefilterUniforms& uniforms);

    Vec4 PrefilterFragmentShader(bool& discard, const PrefilterVaryings& varyings, const PrefilterUniforms& uniforms);

}