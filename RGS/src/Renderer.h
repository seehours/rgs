#pragma once
#include "Base.h"
#include "Framebuffer.h"
#include "Layouts.h"
#include "Math.h"

#include <algorithm>
#include <type_traits>
#include <initializer_list>

#define RGS_MAX_VARYINGS 9

namespace RGS {

    enum class DepthFuncType
    {
        LESS,
        ALWAYS,
    };

    template<typename vertex_t, typename uniforms_t, typename varyings_t>
    struct Program
    {
        bool EnableDoubleSided = false;
        bool EnableBlend = false;
        bool EnableDepthTest = true;
        bool EnableWriteFramebuffer = true;
        bool EnableDrawOutline = false;
        DepthFuncType DepthFunc = DepthFuncType::LESS;

        using vertex_shader_t = void (*)(varyings_t&, const vertex_t&, const uniforms_t&);
        vertex_shader_t VertexShader;

        using fragment_shader_t = Vec4 (*)(bool& discard, const varyings_t&, const uniforms_t&);
        fragment_shader_t FragmentShader;

        Program(vertex_shader_t vertexShader, fragment_shader_t fragmentShader)
            : VertexShader(vertexShader), FragmentShader(fragmentShader) {}
    };

    class Renderer
    {
    public:
        template<typename vertex_t, typename uniforms_t, typename varyings_t>
        static void Draw(Framebuffer* framebuffer, const Program<vertex_t, uniforms_t, varyings_t> *program, const Triangle<vertex_t> *triangle, const uniforms_t *uniforms)
        {
            Draw<vertex_t, uniforms_t, varyings_t>(*framebuffer, *program, *triangle, *uniforms);
        }

        template<typename vertex_t, typename uniforms_t, typename varyings_t>
        static void Draw(Framebuffer& framebuffer, const Program<vertex_t, uniforms_t, varyings_t>& program, const Triangle<vertex_t>& triangle, const uniforms_t& uniforms)
        {
            static_assert(std::is_base_of_v<VertexBase, vertex_t>, "vertex_t 必须继承自 RGS::VertexBase");
            static_assert(std::is_base_of_v<VaryingsBase, varyings_t>, "varyings_t 必须继承自 RGS::VaryingsBase");

            /* Vertex Shading & Projection */ 
            varyings_t varyings[RGS_MAX_VARYINGS];
            for (int i = 0; i < 3; i++)
            {
                program.VertexShader(varyings[i], triangle.Vertex[i], uniforms);
            }

            /* Clipping */
            int vertexNum = Clip(varyings);

            /* Screen Mapping */
            CaculateNdcPos(varyings, vertexNum);
            uint32_t fWidth = framebuffer.GetWidth();
            uint32_t fHeight = framebuffer.GetHeight();
            CaculateFragPos(varyings, vertexNum, fWidth, fHeight);
            
            /* Triangle Assembly & Rasterization */
            for (int i = 0; i < vertexNum - 2; i++)
            {
                varyings_t triVaryings[3];
                triVaryings[0] = varyings[0];
                triVaryings[1] = varyings[i + 1];
                triVaryings[2] = varyings[i + 2];
                bool isCulled = false;
                isCulled = RasterizeTriangle(framebuffer, program, triVaryings, uniforms);
                if (isCulled)
                {
                    break;
                }
            }
        }

    private:
        enum class Plane
        {
            POSITIVE_W,
            POSITIVE_X,
            NEGATIVE_X,
            POSITIVE_Y,
            NEGATIVE_Y,
            POSITIVE_Z,
            NEGATIVE_Z,
        };

        struct BoundingBox { uint32_t MinX, MaxX, MinY, MaxY; };

        static bool IsVertexVisible(const Vec4& clipPos)
        {
            return fabs(clipPos.X) <= clipPos.W && fabs(clipPos.Y) <= clipPos.W && fabs(clipPos.Z) <= clipPos.W;
        }

        static bool IsInsidePlane(const Vec4& clipPos, const Plane plane)
        {
            switch (plane)
            {
            case Plane::POSITIVE_W:
                return clipPos.W >= 0.0f;
            case Plane::POSITIVE_X:
                return clipPos.X <= +clipPos.W;
            case Plane::NEGATIVE_X:
                return clipPos.X >= -clipPos.W;
            case Plane::POSITIVE_Y:
                return clipPos.Y <= +clipPos.W;
            case Plane::NEGATIVE_Y:
                return clipPos.Y >= -clipPos.W;
            case Plane::POSITIVE_Z:
                return clipPos.Z <= +clipPos.W;
            case Plane::NEGATIVE_Z:
                return clipPos.Z >= -clipPos.W;
            default:
                ASSERT(false);
                return false;
            }
        }

        static float GetIntersectRatio(const Vec4& prev, const Vec4& curr, const Plane plane)
        {
            switch (plane) {
            case Plane::POSITIVE_W:
                return (prev.W - 0.0f) / (prev.W - curr.W);
            case Plane::POSITIVE_X:
                //return (prev.W - prev.X) / ((prev.W - prev.X) + (curr.X - curr.W));
                return (prev.W - prev.X) / ((prev.W - prev.X) - (curr.W - curr.X));
            case Plane::NEGATIVE_X:
                //return (prev.W + prev.X) / ((prev.W + prev.X) + (- curr.W - curr.X));
                return (prev.W + prev.X) / ((prev.W + prev.X) - (curr.W + curr.X));
            case Plane::POSITIVE_Y:
                return (prev.W - prev.Y) / ((prev.W - prev.Y) - (curr.W - curr.Y));
            case Plane::NEGATIVE_Y:
                return (prev.W + prev.Y) / ((prev.W + prev.Y) - (curr.W + curr.Y));
            case Plane::POSITIVE_Z:
                return (prev.W - prev.Z) / ((prev.W - prev.Z) - (curr.W - curr.Z));
            case Plane::NEGATIVE_Z:
                return (prev.W + prev.Z) / ((prev.W + prev.Z) - (curr.W + curr.Z));
            default:
                ASSERT(false);
                return 0.0f;
            }
        }

        template <typename varyings_t>
        static void LerpVaryings(varyings_t& out, const varyings_t& start, const varyings_t& end, float ratio)
        {
            constexpr uint32_t floatNum = sizeof(varyings_t) / sizeof(float);
            float* startFloat = (float*)&start;
            float* endFloat = (float*)&end;
            float* outFloat = (float*)&out;

            for (int i = 0; i < floatNum; i++)
            {
                outFloat[i] = Lerp(startFloat[i], endFloat[i], ratio);
            }
        }

        template <typename varyings_t>
        static void LerpVaryings(varyings_t& out, const varyings_t(&varyings)[3], float(&weights)[3])
        {
            constexpr uint32_t floatNum = sizeof(varyings_t) / sizeof(float);
            float* v0 = (float*)&varyings[0];
            float* v1 = (float*)&varyings[1];
            float* v2 = (float*)&varyings[2];
            float* outFloat = (float*)&out;

            for (int i = 0; i < floatNum; i++)
            {
                outFloat[i] = v0[i] * weights[0] + v1[i] * weights[1] + v2[i] * weights[2];
            }
        }

        template<typename varyings_t>
        static int ClipAgainstPlane(varyings_t(&outVaryings)[RGS_MAX_VARYINGS],
                                    const varyings_t(&inVaryings)[RGS_MAX_VARYINGS], 
                                    const Plane plane, 
                                    const int inVertexNum)
        {
            ASSERT(inVertexNum >= 3);

            int outVertexNum = 0;
            for (int i = 0; i < inVertexNum; i++)
            {
                int prevIndex = (i - 1 + inVertexNum) % inVertexNum;
                int currIndex = i;

                const varyings_t& prevVaryings = inVaryings[prevIndex];
                const varyings_t& currVaryings = inVaryings[currIndex];

                const bool prevInside = IsInsidePlane(prevVaryings.ClipPos, plane);
                const bool currInside = IsInsidePlane(currVaryings.ClipPos, plane);

                if (currInside != prevInside)
                {
                    float ratio = GetIntersectRatio(prevVaryings.ClipPos, currVaryings.ClipPos, plane);
                    LerpVaryings(outVaryings[outVertexNum], prevVaryings, currVaryings, ratio);
                    outVertexNum++;
                }

                if (currInside)
                {
                    outVaryings[outVertexNum] = inVaryings[currIndex];
                    outVertexNum++;
                }
            }

            ASSERT(outVertexNum <= RGS_MAX_VARYINGS);
            return outVertexNum;
        }

        template<typename varyings_t>
        static int Clip(varyings_t(&varyings)[RGS_MAX_VARYINGS])
        {
            bool v0_Visible = IsVertexVisible(varyings[0].ClipPos);
            bool v1_Visible = IsVertexVisible(varyings[1].ClipPos);
            bool v2_Visible = IsVertexVisible(varyings[2].ClipPos);
            if (v0_Visible && v1_Visible && v2_Visible)
                return 3;
            
            int vertexNum = 3;
            varyings_t varyings_[RGS_MAX_VARYINGS];
            vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::POSITIVE_W, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_X, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_X, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_Y, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_Y, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_Z, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_Z, vertexNum);
            if (vertexNum == 0) return 0;
            memcpy(varyings, varyings_, sizeof(varyings_));

            return vertexNum;
        }
        
        template<typename varyings_t>
        static void CaculateNdcPos(varyings_t (&varyings)[RGS_MAX_VARYINGS],const int vertexNum)
        {
            for (int i = 0; i < vertexNum; i++)
            {
                float w = varyings[i].ClipPos.W;
                varyings[i].NdcPos = varyings[i].ClipPos / w;
                varyings[i].NdcPos.W = 1.0f / w;
            }
        }

        template<typename varyings_t>
        static void CaculateFragPos(varyings_t(&varyings)[RGS_MAX_VARYINGS], const int vertexNum, const uint32_t width, const uint32_t height)
        {
            for (int i = 0; i < vertexNum; i++)
            { 
                float x = ((varyings[i].NdcPos.X + 1.0f) * 0.5f * (float)width);
                float y = ((varyings[i].NdcPos.Y + 1.0f) * 0.5f * (float)height);
                float z = (varyings[i].NdcPos.Z + 1.0f) * 0.5f;
                float w = varyings[i].NdcPos.W;
                             
                varyings[i].FragPos.X = x;
                varyings[i].FragPos.Y = y;
                varyings[i].FragPos.Z = z;
                varyings[i].FragPos.W = w;
            }
        }

        // 逆时针为正面（可见）
        static bool IsBackFacing(const Vec4& a, const Vec4& b, const Vec4& c)
        {
            // (b.X - a.X) * (c.Y - b.Y) - (b.Y - a.Y) * (c.X - b.X)
            float signedArea = a.X * b.Y - a.Y * b.X +
                               b.X * c.Y - b.Y * c.X +
                               c.X * a.Y - c.Y * a.X;
            return signedArea <= 0;
        }
     
        static BoundingBox GetBoundingBox(const Vec4(&fragCoords)[3], const uint32_t width, const uint32_t height)
        {
            Vec2 screenCoords[3];
            screenCoords[0] = fragCoords[0];
            screenCoords[1] = fragCoords[1];
            screenCoords[2] = fragCoords[2];
            return GetBoundingBox(screenCoords, width, height);
        }
        static BoundingBox GetBoundingBox(const Vec2 (&screenCoords)[3], const uint32_t width, const uint32_t height)
        {
            auto xList = { screenCoords[0].X, screenCoords[1].X, screenCoords[2].X };
            auto yList = { screenCoords[0].Y, screenCoords[1].Y, screenCoords[2].Y };

            float minX = std::min(xList);
            float maxX = std::max(xList);
            float minY = std::min(yList);
            float maxY = std::max(yList);

            minX = Clamp(minX, 0.0f, (float)width - 1.0f);
            maxX = Clamp(maxX, 0.0f, (float)width - 1.0f);
            minY = Clamp(minY, 0.0f, (float)height - 1.0f);
            maxY = Clamp(maxY, 0.0f, (float)height - 1.0f);

            BoundingBox bBox;
            bBox.MinX = (uint32_t)floor(minX);
            bBox.MinY = (uint32_t)floor(minY);
            bBox.MaxX = (uint32_t)ceil(maxX);
            bBox.MaxY = (uint32_t)ceil(maxY);

            return bBox;
        }

        static bool InsideTriangle(const Vec4(&fragCoords)[3], const Vec2& screenPoint)
        {
            Vec2 ap = screenPoint - fragCoords[0];
            Vec2 ab = fragCoords[1] - fragCoords[0];
            float crossAB = Cross({ ap , 0.0f }, { ab, 0.0f }).Z;

            Vec2 bp = screenPoint - fragCoords[1];
            Vec2 bc = fragCoords[2] - fragCoords[1];
            float crossBC = Cross({ bp , 0.0f }, { bc, 0.0f }).Z;

            Vec2 cp = screenPoint - fragCoords[2];
            Vec2 ca = fragCoords[0] - fragCoords[2];
            float crossCA = Cross({ cp , 0.0f }, { ca, 0.0f }).Z;

            if (crossAB >= 0 && crossBC >= 0 && crossCA >= 0)
                return true;
            else if (crossAB <= 0 && crossBC <= 0 && crossCA <= 0)
                return true;
            else
                return false;
        }

        static void CalculateWeights(float(&screenWeights)[3], float(&weights)[3], const Vec4 (&fragCoords)[3], const Vec2& screenPoint)
        {
            Vec2 ab = fragCoords[1] - fragCoords[0];
            Vec2 ac = fragCoords[2] - fragCoords[0];
            Vec2 ap = screenPoint - fragCoords[0];
            float factor = 1 / (ab.X * ac.Y - ab.Y * ac.X);
            float s = (ac.Y * ap.X - ac.X * ap.Y) * factor;
            float t = (ab.X * ap.Y - ab.Y * ap.X) * factor;
            screenWeights[0] = 1 - s - t;
            screenWeights[1] = s;
            screenWeights[2] = t;

            float w0 = fragCoords[0].W * screenWeights[0];
            float w1 = fragCoords[1].W * screenWeights[1];
            float w2 = fragCoords[2].W * screenWeights[2];
            float normalizer = 1.0f / (w0 + w1 + w2);
            weights[0] = w0 * normalizer;
            weights[1] = w1 * normalizer;
            weights[2] = w2 * normalizer;
        }



        static bool PassDepthTest(const float& depth, const float fDepth, const DepthFuncType depthFunc)
        {
            if (depthFunc == DepthFuncType::LESS && depth < fDepth)
            {
                return true;
            }
            else if (depthFunc == DepthFuncType::ALWAYS)
            {
                return true;
            }

            return false;
        }

        template<typename vertex_t, typename uniforms_t, typename varyings_t>
        static void ProcessPixel(Framebuffer& framebuffer, 
                                 const uint32_t x, 
                                 const uint32_t y, 
                                 const Program<vertex_t, uniforms_t, varyings_t>& program, 
                                 const varyings_t& varyings,
                                 const uniforms_t& uniforms)
        {
            /* Pixel Shading */ 
            bool discard = true;
            Vec4 color{ 0.0f, 0.0f, 0.0f, 0.0f };
            color = program.FragmentShader(discard, varyings, uniforms);
            if (discard) 
            { 
                return; 
            }
            color.X = Clamp(color.X, 0.0f, 1.0f);
            color.Y = Clamp(color.Y, 0.0f, 1.0f);
            color.Z = Clamp(color.Z, 0.0f, 1.0f);
            color.W = Clamp(color.W, 0.0f, 1.0f);

            /* Blend */ 
            if (program.EnableBlend)
            {
                Vec3 dstColor = framebuffer.GetColor(x, y);
                Vec3 srcColor = color;
                float alpha = color.W;
                color = { Lerp(dstColor, srcColor, alpha), 1.0f };
                framebuffer.SetColor(x, y, color);
            }
            else
            {
                framebuffer.SetColor(x, y, color);
            }

            if (program.EnableWriteFramebuffer)
            {
                float depth = varyings.FragPos.Z;
                framebuffer.SetDepth(x, y, depth);
            }
        }

        template<typename vertex_t, typename uniforms_t, typename varyings_t>
        static bool RasterizeTriangle(Framebuffer& framebuffer, 
                                      const Program<vertex_t, uniforms_t, varyings_t>& program, 
                                      const varyings_t (&varyings)[3],
                                      const uniforms_t uniforms)
        {
            /* Back Face Culling */
            if (!program.EnableDoubleSided)
            {
                bool isBackFacing = false;
                isBackFacing = IsBackFacing(varyings[0].NdcPos, varyings[1].NdcPos, varyings[2].NdcPos);
                if (isBackFacing)
                {
                    return true;
                }
            }

            /* Bounding Box Setup */
            Vec4 fragCoords[3];
            fragCoords[0] = varyings[0].FragPos;
            fragCoords[1] = varyings[1].FragPos;
            fragCoords[2] = varyings[2].FragPos;
            BoundingBox bBox = GetBoundingBox(fragCoords, framebuffer.GetWidth(), framebuffer.GetHeight());
            /* Triangle Taversal */
            for (uint32_t y = bBox.MinY; y <= bBox.MaxY; y++)
            {
                for (uint32_t x = bBox.MinX; x <= bBox.MaxX; x++)
                {
                    if (x == 50 && y == 180)
                    {
                        int i = 1;
                        i += 1;
                    }
                    /* Varyings Setup */
                    float screenWeights[3];
                    float weights[3];
                    Vec2 screenPoint{ (float)x + 0.5f, (float)y + 0.5f };
                    bool isInsideTriangle = InsideTriangle(fragCoords, screenPoint);
                    if (!isInsideTriangle)
                        continue;

                    CalculateWeights(screenWeights, weights, fragCoords, screenPoint);
                    
                    //if (screenWeights[0] < 0.0f || screenWeights[1] < 0.0f || screenWeights[2] < 0.0f)
                    //{
                    //    continue;
                    //}

                    ///// DEBUG
                    if (false)
                    {
                        constexpr float minor = 0.002f;
                        if (screenWeights[0] < minor || screenWeights[1] < minor || screenWeights[2] < minor)
                        {
                            framebuffer.SetColor(x, y, {1.0f, 1.0f, 1.0f});
                            continue;
                        }
                    }

                    varyings_t pixVaryings;
                    LerpVaryings(pixVaryings, varyings, weights);

                    /* Early Depth Test */ 
                    if (program.EnableDepthTest)
                    {
                        float depth = pixVaryings.FragPos.Z;
                        float fDepth = framebuffer.GetDepth(x, y);
                        DepthFuncType depthFunc = program.DepthFunc;
                        if (!PassDepthTest(depth, fDepth, depthFunc))
                        {
                            continue;
                        }
                    }

                    /* Pixel Processing */ 
                    ProcessPixel(framebuffer, x, y, program, pixVaryings, uniforms);
                }
            }
            return false;
        }
    };

}


