#include "IBLPBRShader.h"

namespace RGS {

    void IBLPBRVertexShader(IBLPBRVaryings& varyings, const IBLPBRVertex& vertex, const IBLPBRUniforms& uniforms)
    {
        varyings.WorldNormal = uniforms.NormalMatrix * Vec4{ vertex.ModelNormal, 0.0f };
        varyings.TexPos = vertex.ModelPos;
        varyings.WorldPos = uniforms.ModelMatrix * vertex.ModelPos;

        varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
    }

    // ----------------------------------------------------------------------------
    static float DistributionGGX(Vec3 N, Vec3 H, float roughness)
    {
        float a = roughness * roughness;
        float a2 = a * a;
        float NDotH = Max(Dot(N, H), 0.0);
        float NDotH2 = NDotH * NDotH;

        float nom = a2;
        float denom = (NDotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    static float GeometrySchlickGGX(float NDotV, float roughness)
    {
        float r = (roughness + 1.0);
        float k = (r * r) / 8.0;

        float nom = NDotV;
        float denom = NDotV * (1.0 - k) + k;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    static float GeometrySmith(Vec3 N, Vec3 V, Vec3 L, float roughness)
    {
        float NDotV = Max(Dot(N, V), 0.0);
        float NDotL = Max(Dot(N, L), 0.0);
        float ggx2 = GeometrySchlickGGX(NDotV, roughness);
        float ggx1 = GeometrySchlickGGX(NDotL, roughness);

        return ggx1 * ggx2;
    }
    // ----------------------------------------------------------------------------
    static Vec3 fresnelSchlick(float cosTheta, Vec3 F0)
    {
        return F0 + (1.0 - F0) * pow(Clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }
    // ----------------------------------------------------------------------------
    static Vec3 fresnelSchlickRoughness(float cosTheta, Vec3 F0, float roughness)
    {
        Vec3 v = Vec3(1.0 - roughness);
        v.X = Max(v.X, F0.X);
        v.Y = Max(v.Y, F0.Y);
        v.Z = Max(v.Z, F0.Z);
        Vec3 color = F0 + ((v - F0) * std::pow(Clamp(1.0 - cosTheta, 0.0f, 1.0f), 5.0f));
        return color;
    }

    Vec4 IBLPBRFragmentShader(bool& discard, const IBLPBRVaryings& varyings, const IBLPBRUniforms& uniforms)
    {
        Vec3 N = Normalize(varyings.WorldNormal);
        Vec3 V = Normalize(uniforms.CamPos - varyings.WorldPos);
        Vec3 R = Reflect(-1 * V, N);

        float roughness = uniforms.Roughness;

        // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
        // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
        Vec3 F0 = Vec3(0.04f);
        F0 = Lerp(F0, uniforms.Albedo, uniforms.Metallic);

        // reflectance equation
        Vec3 Lo = Vec3(0.0f);

#if false
        {
            // calculate per-light radiance
            Vec3 L = Normalize(uniforms.LightPos - varyings.WorldPos);
            Vec3 H = Normalize(V + L);
            float distance = Length(uniforms.LightPos - varyings.WorldPos);
            float attenuation = 1.0 / (distance * distance);
            Vec3 radiance = uniforms.LightColor * attenuation;

            // Cook-Torrance BRDF
            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            Vec3 F = fresnelSchlick(Max(Dot(H, V), 0.0), F0);

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

            // scale light by NDotL
            float NDotL = Max(Dot(N, L), 0.0);

            // add to outgoing radiance Lo
            Lo += (kD * uniforms.Albedo / PI + specular) * radiance * NDotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        }

#endif //  0

        // ambient lighting (we now use IBL as the ambient term)
        float NoV = Dot(N, V);
        Vec3 F = fresnelSchlickRoughness(Max(NoV, 0.0), F0, roughness);

        Vec3 kS = F;
        Vec3 kD = 1.0 - kS;
        kD *= 1.0 - uniforms.Metallic;

        Vec3 irradiance = uniforms.IrradianceMap->Sample(varyings.TexPos);
        Vec3 diffuse = irradiance * uniforms.Albedo;

        // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
        constexpr float Max_REFLECTION_LOD = 4.0;
        Vec3 prefilteredColor = uniforms.PrefilterMap->Sample(R, roughness * 4.0f); 
        Vec2 brdf = uniforms.BrdfLUT->Sample( Vec2(Max(NoV, 0.0), roughness));
        Vec3 specular = prefilteredColor * (F * brdf.X + brdf.Y);

        Vec3 ambient = (kD * diffuse + specular)* uniforms.Ao;

        Vec3 color = ambient + Lo;

        // HDR tonemapping
        color = color / (color + Vec3(1.0f));
        // gamma correct
        constexpr float gamma = 1.0f / 2.2f;
        color = Pow(color, gamma);

        return Vec4{ color, 1.0f };
    }

}