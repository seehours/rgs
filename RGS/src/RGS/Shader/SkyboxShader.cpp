#include "SkyboxShader.h"

#define FOR_EVERY_DIMENSION(vec3, func_, ...) \
    do { \
        vec3##.X = func_((vec3##.X), __VA_ARGS__ );\
        vec3##.Y = func_((vec3##.Y), __VA_ARGS__ );\
        vec3##.Z = func_((vec3##.Z), __VA_ARGS__ );\
    } while(0)

namespace RGS {

    void SkyboxVertexShader(SkyboxVaryings& varyings, const SkyboxVertex& vertex, const SkyboxUniforms& uniforms)
    {
        varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
        varyings.ClipPos.Z = varyings.ClipPos.W;
        varyings.TexPos = vertex.ModelPos;
    }

    Vec4 SkyboxFragmentShader(bool& discard, const SkyboxVaryings& varyings, const SkyboxUniforms& uniforms)
    {
        discard = false;
        Vec3 envColor;
        if (uniforms.SkyboxTex != nullptr)
        {
            envColor = uniforms.SkyboxTex->Sample(Normalize(varyings.TexPos));
        }
        else if (uniforms.LodSkyboxTex != nullptr)
        {
            envColor = uniforms.LodSkyboxTex->Sample(Normalize(varyings.TexPos), uniforms.Lod);
        }
        envColor = envColor / (envColor + Vec3{ 1.0f });
        constexpr float gamma = 1.0f / 2.2f;
        envColor = Pow(envColor, gamma);
        return { envColor , 1.0f};
    }

}