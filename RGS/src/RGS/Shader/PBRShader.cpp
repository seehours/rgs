#include "PBRShader.h"

namespace RGS {

    void PBRVertexShader(PBRVaryings& varyings, const PBRVertex& vertex, const PBRUniforms& uniforms)
    {
        varyings.WorldPos = uniforms.Model * vertex.ModelPos;
        varyings.WorldNormal = uniforms.ModelNormalToWorld * Vec4{ vertex.Normal, 0.0f };

        varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
    }

    // ----------------------------------------------------------------------------
    static float DistributionGGX(Vec3 N, Vec3 H, float roughness)
    {
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = Max(Dot(N, H), 0.0f);
        float NdotH2 = NdotH * NdotH;

        float nom = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }

    // ----------------------------------------------------------------------------
    static float GeometrySchlickGGX(float NdotV, float roughness)
    {
        float r = (roughness + 1.0);
        float k = (r * r) / 8.0;

        float nom = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / denom;
    }

    // ----------------------------------------------------------------------------
    static float GeometrySmith(Vec3 N, Vec3 V, Vec3 L, float roughness)
    {
        float NdotV = Max(Dot(N, V), 0.0f);
        float NdotL = Max(Dot(N, L), 0.0f);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    // ----------------------------------------------------------------------------
    static Vec3 fresnelSchlick(float cosTheta, Vec3 F0)
    {
        return F0 + (Vec3{ 1.0f, 1.0f, 1.0f } - F0) * pow(Clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    static Vec3 GammaCorrection(const Vec3& v)
    {
        float x = pow(v.X, 1.0f / 2.2f);
        float y = pow(v.Y, 1.0f / 2.2f);
        float z = pow(v.Z, 1.0f / 2.2f);
        return { x, y, z };
    }

    Vec4 PBRFragmentShader(bool& discard, const PBRVaryings& varyings, const PBRUniforms& uniforms)
    {

        Vec3 N = Normalize(varyings.WorldNormal);
        Vec3 V = Normalize(uniforms.CamPos - varyings.WorldPos);

        // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
        // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
        Vec3 F0 = Vec3(0.04f);
        F0 = Lerp(F0, uniforms.Albedo, uniforms.Metallic);

        // reflectance equation
        Vec3 Lo = Vec3(0.0);
        for (int i = 0; i < 4; ++i)
        {
            // calculate per-light radiance
            Vec3 L = Normalize(uniforms.LightPositions[i] - varyings.WorldPos);
            Vec3 H = Normalize(V + L);
            float distance = Length(uniforms.LightPositions[i] - varyings.WorldPos);
            float attenuation = 1.0 / (distance * distance);
            Vec3 radiance = uniforms.LightColors[i] * attenuation;

            // Cook-Torrance BRDF
            float NDF = DistributionGGX(N, H, uniforms.Roughness);
            float G = GeometrySmith(N, V, L, uniforms.Roughness);
            Vec3 F = fresnelSchlick(Clamp(Dot(H, V), 0.0, 1.0), F0);

            Vec3 numerator = NDF * G * F;
            float denominator = 4.0 * Max(Dot(N, V), 0.0) * Max(Dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
            Vec3 specular = numerator / denominator;

            // kS is equal to Fresnel
            Vec3 kS = F;
            // for energy conservation, the diffuse and specular light can't
            // be above 1.0 (unless the surface emits light); to preserve this
            // relationship the diffuse component (kD) should equal 1.0 - kS.
            Vec3 kD = Vec3(1.0) - kS;
            // multiply kD by the inverse metalness such that only non-metals 
            // have diffuse lighting, or a linear blend if partly metal (pure metals
            // have no diffuse light).
            kD *= 1.0 - uniforms.Metallic;

            // scale light by NdotL
            float NdotL = Max(Dot(N, L), 0.0);

            // add to outgoing radiance Lo
            Lo += (kD * uniforms.Albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        }

        // ambient lighting (note that the next IBL tutorial will replace 
        // this ambient lighting with environment lighting).
        Vec3 ambient = Vec3(0.03) * uniforms.Albedo * uniforms.Ao;

        Vec3 color = ambient + Lo;

        // HDR tonemapping
        color = color / (color + Vec3(1.0));
        // gamma correct

        discard = false;
        return Vec4(color, 1.0);
    }


}