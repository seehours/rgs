#include "BRDFShader.h"

namespace RGS {

    void BRDFVertexShader(BRDFVaryings& varyings, const BRDFVertex& vertex, const BRDFUniforms& uniforms)
    {
        varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
    }

    // ----------------------------------------------------------------------------
    // http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
    // efficient VanDerCorpus calculation.
    static float RadicalInverse_VdC(uint32_t bits)
    {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return float(bits) * 2.3283064365386963e-10; // / 0x100000000
    }
    // ----------------------------------------------------------------------------
    static Vec2 Hammersley(uint32_t i, uint32_t N)
    {
        return Vec2(float(i) / float(N), RadicalInverse_VdC(i));
    }
    // ----------------------------------------------------------------------------
    static Vec3 ImportanceSampleGGX(Vec2 Xi, Vec3 N, float roughness)
    {
        float a = roughness * roughness;

        float phi = 2.0 * PI * Xi.X;
        float cosTheta = sqrt((1.0 - Xi.Y) / (1.0 + (a * a - 1.0) * Xi.Y));
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        // from spherical coordinates to cartesian coordinates - halfway Vector
        Vec3 H;
        H.X = cos(phi) * sinTheta;
        H.Y = sin(phi) * sinTheta;
        H.Z = cosTheta;

        // from tangent-space H Vector to world-space sample Vector
        Vec3 up = abs(N.Z) < 0.999 ? Vec3(0.0, 0.0, 1.0) : Vec3(1.0, 0.0, 0.0);
        Vec3 tangent = Normalize(Cross(up, N));
        Vec3 bitangent = Cross(N, tangent);

        Vec3 sampleVec = tangent * H.X + bitangent * H.Y + N * H.Z;
        return Normalize(sampleVec);
    }
    // ----------------------------------------------------------------------------
    static float GeometrySchlickGGX(float NdotV, float roughness)
    {
        // note that we use a different k for IBL
        float a = roughness;
        float k = (a * a) / 2.0;

        float nom = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / denom;
    }
    // ----------------------------------------------------------------------------
    static float GeometrySmith(Vec3 N, Vec3 V, Vec3 L, float roughness)
    {
        float NdotV = Max(Dot(N, V), 0.0);
        float NdotL = Max(Dot(N, L), 0.0);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }
    // ----------------------------------------------------------------------------
    static Vec2 IntegrateBRDF(float NdotV, float roughness)
    {
        Vec3 V;
        V.X = sqrt(1.0 - NdotV * NdotV);
        V.Y = 0.0f;
        V.Z = NdotV;

        float A = 0.0f;
        float B = 0.0f;

        Vec3 N = Vec3(0.0, 0.0, 1.0);

        constexpr uint32_t SAMPLE_COUNT = 1024u;
        for (uint32_t i = 0u; i < SAMPLE_COUNT; ++i)
        {
            // generates a sample Vector that's biased towards the
            // preferred alignment direction (importance sampling).
            Vec2 Xi = Hammersley(i, SAMPLE_COUNT);
            Vec3 H = ImportanceSampleGGX(Xi, N, roughness);
            Vec3 L = Normalize(2.0 * Dot(V, H) * H - V);

            float NdotL = Max(L.Z, 0.0);
            float NdotH = Max(H.Z, 0.0);
            float VdotH = Max(Dot(V, H), 0.0);

            if (NdotL > 0.0)
            {
                float G = GeometrySmith(N, V, L, roughness);
                float G_Vis = (G * VdotH) / (NdotH * NdotV);
                float Fc = pow(1.0 - VdotH, 5.0);

                A += (1.0 - Fc) * G_Vis;
                B += Fc * G_Vis;
            }
        }
        A /= float(SAMPLE_COUNT);
        B /= float(SAMPLE_COUNT);
        return Vec2{ A, B };
    }

    Vec4 BRDFFragmentShader(bool& discard, const BRDFVaryings& varyings, const BRDFUniforms& uniforms)
    {
        discard = false;

        float NoV = varyings.NdcPos.X * 0.5f + 0.5f;
        float roughness = varyings.NdcPos.Y * 0.5f + 0.5f;

        Vec2 integratedBRDF = IntegrateBRDF(NoV, roughness);

        return { integratedBRDF, 0.0f, 0.0f };
    }

}