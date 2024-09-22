#pragma once 
#include "ShaderBase.h"

namespace RGS {

    using IBLPBRVertex = VertexBase3D;

    struct IBLPBRVaryings : public VaryingsBase
    {
        Vec3 TexPos = { 0.0f ,0.0f, 0.0f };
        Vec3 WorldNormal;
        Vec3 WorldPos;
    };

    struct IBLPBRUniforms : public UniformsBase
    {
        Mat4 NormalMatrix;
        Mat4 ModelMatrix;
        Vec3 CamPos;

        Vec3 Albedo;
        float Metallic;
        float Roughness;
        float Ao;

        Vec3 LightPos;
        Vec3 LightColor;

        TextureSphere* IrradianceMap;
        LodTextureSphere* PrefilterMap;
        Texture* BrdfLUT;
    };

    void IBLPBRVertexShader(IBLPBRVaryings& varyings, const IBLPBRVertex& vertex, const IBLPBRUniforms& uniforms);

    Vec4 IBLPBRFragmentShader(bool& discard, const IBLPBRVaryings& varyings, const IBLPBRUniforms& uniforms);

}
