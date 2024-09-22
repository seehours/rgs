#include "ConvSkyShader.h"

namespace RGS {

    static Vec3 static_UV2Vec3(Vec2 uv)
    {
        Vec3 res{ 0.0f };
        float phi = uv.X * 2 * PI - PI;
        float theta = (1.0f - uv.Y) * PI;
        res.Z = sin(phi) * sin(theta);
        res.X = cos(phi) * sin(theta);
        res.Y = cos(theta);

        return res;
    }

    void ConvSkyVertexShader(ConvSkyVaryings& varyings, const ConvSkyVertex& vertex, const ConvSkyUniforms& uniforms)
    {
        varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
    }

    Vec4 ConvSkyFragmentShader(bool& discard, const ConvSkyVaryings& varyings, const ConvSkyUniforms& uniforms)
    {
        discard = false;

        Vec2 uv = { varyings.NdcPos.X / 2.0f + 0.5f, varyings.NdcPos.Y / 2.0f + 0.5f };

        Vec3 worldY = Normalize(static_UV2Vec3(uv));
        
        Vec3 irradiance = Vec3(0.0);

        Vec3 worldX = Vec3(1.0, 0.0, 0.0);
        Vec3 worldZ = Normalize(Cross(worldX, worldY));
        worldX = Normalize(Cross(worldY, worldZ));


        float sampleDelta = 0.025;
        float nrSamples = 0.0;
        for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
        {
            for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
            {
                // spherical to cartesian (in tangent space)
                Vec3 tangentSample = Vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
                // tangent space to world
                Vec3 sampleVec = tangentSample.X * worldX + tangentSample.Y * worldY + tangentSample.Z * worldZ;

                irradiance += uniforms.SkyboxTex->Sample(sampleVec) * cos(theta) * sin(theta);
                nrSamples++;
            }
        }
        irradiance = PI * irradiance * (1.0 / float(nrSamples)); 

        //constexpr float gamma = 1.0f / 2.2f;
        //irradiance.X = pow(irradiance.X, gamma);
        //irradiance.Y = pow(irradiance.Y, gamma);
        //irradiance.Z = pow(irradiance.Z, gamma);

        return { irradiance , 1.0f };
    }


    void PrefilterVertexShader(PrefilterVaryings& varyings, const PrefilterVertex& vertex, const PrefilterUniforms& uniforms)
    {
        varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
    }


    static float DistributionGGX(Vec3 N, Vec3 H, float roughness)
    {
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = Max(Dot(N, H), 0.0);
        float NdotH2 = NdotH * NdotH;

        float nom = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
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

    Vec4 PrefilterFragmentShader(bool& discard, const PrefilterVaryings& varyings, const PrefilterUniforms& uniforms)
    {
        discard = false;
        float roughness = uniforms.Roughness;

        Vec2 uv = { varyings.NdcPos.X / 2.0f + 0.5f, varyings.NdcPos.Y / 2.0f + 0.5f };

        Vec3 N = Normalize(static_UV2Vec3(uv));

        // make the simplifying assumption that V equals R equals the normal 
        Vec3 R = N;
        Vec3 V = R;

        constexpr uint32_t SAMPLE_COUNT = 1024u;
        Vec3 prefilteredColor = Vec3{ 0.0f };
        float totalWeight = 0.0;

        for (uint32_t i = 0u; i < SAMPLE_COUNT; ++i)
        {
            // generates a sample Vector that's biased towards the preferred alignment direction (importance sampling).
            Vec2 Xi = Hammersley(i, SAMPLE_COUNT);
            Vec3 H = ImportanceSampleGGX(Xi, N, roughness);
            Vec3 L = Normalize(2.0 * Dot(V, H) * H - V);

            float NdotL = Max(Dot(N, L), 0.0);
            if (NdotL > 0.0)
            {
                // sample from the environment's mip level based on roughness/pdf
                float D = DistributionGGX(N, H, roughness);
                float NdotH = Max(Dot(N, H), 0.0);
                float HdotV = Max(Dot(H, V), 0.0);
                float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

                //float resolution = 512.0; // resolution of source cubemap (per face)
                //float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
                //float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

                //float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
                float mipLevel = roughness * 4.0f;

                //prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
                prefilteredColor += uniforms.SkyboxTex->Sample(L, mipLevel) * NdotL;
                totalWeight += NdotL;
            }
        }

        prefilteredColor = prefilteredColor / totalWeight;

        return { prefilteredColor, 1.0f };

    }
}