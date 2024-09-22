#include "BlinnShader.h"

namespace RGS {

    void BlinnVertexShader(BlinnVaryings& varyings, const BlinnVertex& vertex, const BlinnUniforms& uniforms)
    {
        varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
        varyings.WorldPos = uniforms.Model * vertex.ModelPos;
        varyings.WorldNormal = uniforms.ModelNormalToWorld * Vec4(vertex.ModelNormal, 0.0f);
        varyings.TexCoord = vertex.TexCoord;
    }

    Vec4 BlinnFragmentShader(bool& discard, const BlinnVaryings& varyings, const BlinnUniforms& uniforms)
    {
        discard = false;

        const Vec3& cameraPos = uniforms.CameraPos;
        const Vec3& lightPos = uniforms.LightPos;
        const Vec3& worldPos = varyings.WorldPos;
        Vec3 worldNormal = Normalize(varyings.WorldNormal);
        Vec3 viewDir = Normalize(cameraPos - worldPos);
        Vec3 lightDir = Normalize(lightPos - worldPos);

        Vec3 halfDir = Normalize(lightDir + viewDir);

        Vec3 _Color{ 0.5f, 0.5f, 0.5f };
        float _Speclur = 8.0f;
        if (uniforms.Diffuse && uniforms.Specular)
        {
            Vec2 texCoord = varyings.TexCoord;
            _Color = uniforms.Diffuse->Sample(texCoord) * 0.7f;
            _Speclur = uniforms.Specular->Sample(texCoord).X * 16.0f;
        }

        Vec3 ambient = _Color * 0.5f;
        Vec3 diffuse = std::max(0.0f, Dot(worldNormal, lightDir)) * _Color;
        Vec3 speculur = (float)pow(std::max(0.0f, Dot(halfDir, worldNormal)), _Speclur) * _Color;

        return { ambient + diffuse + speculur, 1.0f };
    }


}