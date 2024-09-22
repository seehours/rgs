#pragma once
#include "RGS/Config.h"

#include "Mesh.h"
#include "Framebuffer.h"

#include "RGS/Base/Base.h"
#include "RGS/Base/Maths.h"
#include "RGS/Shader/ShaderBase.h"
#include "RGS/JobSystem.h"

#include <algorithm>
#include <type_traits>
#include <initializer_list>
#include <memory>

namespace RGS {

    struct Camera
    {
        Vec4 Pos = { 0.0f, 0.0f, 0.0f, 1.0f };
        Vec4 Right = { 1.0f, 0.0f, 0.0f, 0.0f };
        Vec4 Up = { 0.0f, 1.0f, 0.0f, 0.0f };
        Vec4 Dir = { 0.0f, 0.0f, -1.0f, 0.0f };

        float Aspect = 4.0f / 3.0f;

        Mat4 ViewMat4() const
        {
            if (Dir.Y >= 0.999f || Dir.Y <= -0.999f)
            {
                return Mat4LookAt(Pos, Pos + Dir, { 1.0f, 0.0f, 0.0f });
            }
            else
            {
                return Mat4LookAt(Pos, Pos + Dir, { 0.0f, 1.0f, 0.0f });
            }
        }
        Mat4 ProjectionMat4() const
        {
            return Mat4Perspective(90.0f / 360.0f * 2.0f * PI, Aspect, 0.1f, 100.0f);
        }
    };

    enum class DepthFuncType
    {
        LESS,
        LEQUAL,
        ALWAYS,
    };

    template<typename vertex_t, typename uniforms_t, typename varyings_t>
    struct Program
    {
        bool EnableDoubleSided = false;
        bool EnableBlend = false;
        bool EnableDepthTest = true;
        bool EnableWriteDepth = true;
        bool EnableJobSystem = true;

        DepthFuncType DepthFunc = DepthFuncType::LESS;

        using vertex_shader_t = void (*)(varyings_t&, const vertex_t&, const uniforms_t&);
        vertex_shader_t VertexShader;

        using fragment_shader_t = Vec4(*)(bool& discard, const varyings_t&, const uniforms_t&);
        fragment_shader_t FragmentShader;

        Program(const vertex_shader_t vertexShader, const fragment_shader_t fragmentShader)
            : VertexShader(vertexShader), FragmentShader(fragmentShader) {}
    };

    class Renderer
    {
    public:
        inline static uint32_t s_FaceCount = 0;

    private:
        static constexpr int RGS_MAX_VARYINGS = 9;

    private:
        enum class Plane
        {
            POSITIVE_X,
            NEGATIVE_X,
            POSITIVE_Y,
            NEGATIVE_Y,
            POSITIVE_Z,
            NEGATIVE_Z,
        };

        struct ScreenWeights { float W[3]; float Padding; };

        struct Weights { float W[3]; float Padding; };

        struct BoundingBox { int MinX, MaxX, MinY, MaxY; };

        static bool IsVertexVisible(const Vec4& clipPos);
        static bool IsInsidePlane(const Vec4& clipPos, const Plane plane);
        static bool IsInsideTriangle(float(&weights)[3]);
        static bool IsBackFacing(const Vec4& a, const Vec4& b, const Vec4& c);
        static bool PassDepthTest(const float writeDepth, const float fDepth, const DepthFuncType depthFunc);

        static float GetIntersectRatio(const Vec4& prev, const Vec4& curr, const Plane plane);
        static BoundingBox GetBoundingBox(const Vec4(&fragCoords)[3], const int width, const int height);

        static void CalculateWeights(ScreenWeights& screenWeights, 
                                     Weights &weights,
                                     const Vec4(&fragCoords)[3], 
                                     const Vec2& screenPoint);
       
        template <typename varyings_t>
        static void LerpVaryings(varyings_t& out, 
                                 const varyings_t& start, 
                                 const varyings_t& end, 
                                 const float ratio)
        {
            constexpr uint32_t floatNum = sizeof(varyings_t) / sizeof(float);
            float* startFloat = (float*)&start;
            float* endFloat = (float*)&end;
            float* outFloat = (float*)&out;

            for (int i = 0; i < (int)floatNum; i++)
            {
                outFloat[i] = Lerp(startFloat[i], endFloat[i], ratio);
            }
        }

        template <typename varyings_t>
        static void LerpVaryings(varyings_t& out, 
                                 const varyings_t(&varyings)[3], 
                                 const float(&weights)[3], 
                                 const uint32_t width, 
                                 const uint32_t height)
        {
            out.ClipPos = varyings[0].ClipPos * weights[0] +
                varyings[1].ClipPos * weights[1] +
                varyings[2].ClipPos * weights[2];

            out.NdcPos = out.ClipPos / out.ClipPos.W;
            out.NdcPos.W = 1.0f / out.ClipPos.W;

            out.FragPos.X = ((out.NdcPos.X + 1.0f) * 0.5f * width);
            out.FragPos.Y = ((out.NdcPos.Y + 1.0f) * 0.5f * height);
            out.FragPos.Z = (out.NdcPos.Z + 1.0f) * 0.5f;
            out.FragPos.W = out.NdcPos.W;

            constexpr uint32_t floatOffset = sizeof(Vec4) * 3 / sizeof(float);
            constexpr uint32_t floatNum = sizeof(varyings_t) / sizeof(float);
            float* v0 = (float*)&varyings[0];
            float* v1 = (float*)&varyings[1];
            float* v2 = (float*)&varyings[2];
            float* outFloat = (float*)&out;

            for (int i = floatOffset; i < (int)floatNum; i++)
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
            vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::NEGATIVE_Z, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::POSITIVE_Z, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::POSITIVE_X, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::NEGATIVE_X, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings_, varyings, Plane::POSITIVE_Y, vertexNum);
            if (vertexNum == 0) return 0;
            vertexNum = ClipAgainstPlane(varyings, varyings_, Plane::NEGATIVE_Y, vertexNum);
            if (vertexNum == 0) return 0;

            return vertexNum;
        }

        template<typename varyings_t>
        static void CaculateNdcPos(varyings_t(&varyings)[RGS_MAX_VARYINGS], const int vertexNum)
        {
            for (int i = 0; i < vertexNum; i++)
            {
                float w = varyings[i].ClipPos.W;
                varyings[i].NdcPos = varyings[i].ClipPos / w;
                varyings[i].NdcPos.W = 1.0f / w;
            }
        }

        template<typename varyings_t>
        static void CaculateFragPos(varyings_t(&varyings)[RGS_MAX_VARYINGS], 
                                    const int vertexNum, 
                                    const float width, 
                                    const float height)
        {
            for (int i = 0; i < vertexNum; i++)
            {
                float x = ((varyings[i].NdcPos.X + 1.0f) * 0.5f * width);
                float y = ((varyings[i].NdcPos.Y + 1.0f) * 0.5f * height);
                float z = (varyings[i].NdcPos.Z + 1.0f) * 0.5f;
                float w = varyings[i].NdcPos.W;

                varyings[i].FragPos.X = x;
                varyings[i].FragPos.Y = y;
                varyings[i].FragPos.Z = z;
                varyings[i].FragPos.W = w;
            }
        }

        template<typename vertex_t, typename uniforms_t, typename varyings_t, MSAA msaa>
        static void ProcessPixel(Framebuffer& framebuffer,
                                 const int x,
                                 const int y,
                                 const Program<vertex_t, uniforms_t, varyings_t>& program,
                                 const varyings_t& varyings,
                                 const uniforms_t& uniforms,
                                 const bool(&coverage)[(int)msaa],
                                 const bool(&depthOcclusion)[(int)msaa],
                                 const bool isEdge)
        {
            /* Pixel Shading */
            bool discard = false;
            Vec4 color{ 0.0f, 0.0f, 0.0f, 0.0f };
            color = program.FragmentShader(discard, varyings, uniforms);
            if (discard)
            {
                return;
            }
            color = Clamp(color, 0.0f, 1.0f);

            /* Blend */
            if (program.EnableBlend)
            {
                for (int i = 0; i < (int)msaa; ++i)
                {   
                    if (coverage[i] && !depthOcclusion[i])
                    {
                        Vec3 dstColor = framebuffer.GetColor(x, y, i);
                        Vec3 srcColor = (Vec3)color;
                        float alpha = color.W;
                        dstColor = { Lerp(dstColor, srcColor, alpha) };

                        framebuffer.SetColor(x, y, i, dstColor);
                    }
                }   
            }
            else
            {
                for (int i = 0; i < (int)msaa; ++i)
                {
                    if (coverage[i] && !depthOcclusion[i])
                    {
                       framebuffer.SetColor(x, y, i, color);
                    }
                }
            }

            if (program.EnableWriteDepth)
            {
                for (int i = 0; i < (int)msaa; ++i)
                {
                    if (coverage[i] && !depthOcclusion[i])
                    {
                        float depth = varyings.FragPos.Z;
                        framebuffer.SetDepth(x, y, i, depth);
                    }
                }
            }

#if RGS_ENABLE_WIREFRAME_MODE

            if (isEdge) 
            {
                for (int i = 0; i < (int)msaa ; ++i)
                {
                    if (coverage[i] && !depthOcclusion[i])
                    {
                        Vec3 color = Pow(framebuffer.GetColor(x, y, i), 0.65f) ;
                        framebuffer.SetColor(x, y, i, color);
                    }
                }
            }

#endif //  RGS_ENABLE_WIREFRAME_MODE
        }

        template<typename vertex_t, typename uniforms_t, typename varyings_t, MSAA msaa>
        static void SetupAndProcessPixel(Framebuffer& framebuffer,
                                         const int x, 
                                         const int y, 
                                         const Program<vertex_t, uniforms_t, varyings_t>& program,
                                         const varyings_t(&varyings)[3],
                                         const uniforms_t& uniforms,
                                         const Vec4(&fragCoords)[3], 
                                         uint32_t fWidth, 
                                         uint32_t fHeight)
        {
            /* Varyings Setup */
            ScreenWeights screenWeights;
            Weights weights;

            bool coverage[(int)msaa];
            bool isInsideTriangle = false;
            bool isOutsideTriangle = false;
            Vec2 coverPoint;
            for (int i = 0; i < (int)msaa; ++i)
            {
                coverPoint = Vec2{ (float)x , (float)y } + MSAA_SampleTable::GetSamplePoints<msaa>()[i];
                CalculateWeights(screenWeights, weights, fragCoords, coverPoint);
                if (IsInsideTriangle(weights.W))
                {
                    isInsideTriangle = true;
                    coverage[i] = true;
                }
                else
                {
                    coverage[i] = false;
                    isOutsideTriangle = true;
                }
            }

#if RGS_ENABLE_WIREFRAME_MODE
            if constexpr (msaa == MSAA::None)
            {
                for (int i = 0; i < 2; ++i)
                {
                    Vec2 coverPoint = Vec2{ (float)x , (float)y } + MSAA_SampleTable::GetSamplePoints<2>()[i];
                    CalculateWeights(screenWeights, weights, fragCoords, coverPoint);
                    if (IsInsideTriangle(weights))
                    {
                        isInsideTriangle = true;
                    }
                    else
                    {
                        isOutsideTriangle = true;
                    }
                }
            }
#endif

            if (!isInsideTriangle)
                return;

            Vec2 screenPoint{ (float)x + 0.5f, (float)y + 0.5f };
            CalculateWeights(screenWeights, weights, fragCoords, screenPoint);
            varyings_t pixVaryings;
            LerpVaryings(pixVaryings, varyings, weights.W, fWidth, fHeight);

            /* Early Depth Test */
            bool depthOcclusion[(int)msaa];
            if (program.EnableDepthTest)
            {
                float depth = pixVaryings.FragPos.Z;
                DepthFuncType depthFunc = program.DepthFunc;

                for (int i = 0; i < (int)msaa; ++i)
                {
                    float fDepth = framebuffer.GetDepth(x, y, i);
                    depthOcclusion[i] = !PassDepthTest(depth, fDepth, depthFunc);
                }
            }
            else
            {
                for (int i = 0; i < (int)msaa; ++i)
                {
                    depthOcclusion[i] = false;
                }
            }

            /* Pixel Processing */
            ProcessPixel<vertex_t, uniforms_t, varyings_t, msaa>(
                framebuffer, x, y, program, pixVaryings, uniforms, coverage, depthOcclusion, isOutsideTriangle); // isOutside && isInside => edge
        }

        template<typename vertex_t, typename uniforms_t, typename varyings_t, MSAA msaa>
        static void RasterizeTriangle(Framebuffer& framebuffer,
                                      const Program<vertex_t, uniforms_t, varyings_t>& program,
                                      const varyings_t(&varyings)[3],
                                      const uniforms_t& uniforms)
        {
            /* Back Face Culling */
            if (!program.EnableDoubleSided)
            {
                bool isBackFacing = false;
                isBackFacing = IsBackFacing(varyings[0].NdcPos, varyings[1].NdcPos, varyings[2].NdcPos);
                if (isBackFacing)
                {
                    return;
                }
            }

            /* Bounding Box Setup */
            Vec4 fragCoords[3];
            fragCoords[0] = varyings[0].FragPos;
            fragCoords[1] = varyings[1].FragPos;
            fragCoords[2] = varyings[2].FragPos;
            uint32_t fWidth = framebuffer.GetWidth();
            uint32_t fHeight = framebuffer.GetHeight();
            BoundingBox bBox = GetBoundingBox(fragCoords, fWidth, fHeight);

            /* Triangle Traversal */
            if (program.EnableJobSystem)
            {
                int minX = bBox.MinX;
                int minY = bBox.MinY;
                uint32_t bWidth = bBox.MaxX - minX + 1u;
                uint32_t bHeight = bBox.MaxY - minY + 1u;
                uint32_t jobCount = bWidth * bHeight;

                //TODO: Config
                constexpr uint32_t groupSize = 2048u;
                JobSystem::Dispatch(jobCount, groupSize, [=, &framebuffer](JobSystem::JobDispatchArgs args)
                {
                    int x = args.JobIndex % bWidth + minX;
                    int y = args.JobIndex / bWidth + minY;
                    SetupAndProcessPixel<vertex_t, uniforms_t, varyings_t, msaa>(
                        framebuffer, x, y, program, varyings, uniforms, fragCoords, fWidth, fHeight);

                });
            }
            else // Single-threaded for loop
            {
                for (int y = bBox.MinY; y <= bBox.MaxY; y++)
                {
                    for (int x = bBox.MinX; x <= bBox.MaxX; x++)
                    {
                        SetupAndProcessPixel<vertex_t, uniforms_t, varyings_t, msaa>(
                            framebuffer, x, y, program, varyings, uniforms, fragCoords, fWidth, fHeight);

                    }
                }
            }
        }

    public:

        template<typename vertex_t, typename uniforms_t, typename varyings_t, MSAA msaa = MSAA::None>
        static void DrawTriangle(Framebuffer& framebuffer,
                                 std::shared_ptr<Program<vertex_t, uniforms_t, varyings_t>> program,
                                 const Triangle<vertex_t>& triangle,
                                 std::shared_ptr<uniforms_t> uniforms)
        {
            s_FaceCount++;
            static_assert(std::is_base_of_v<VertexBase, vertex_t>, "vertex_t 必须继承自 RGS::VertexBase");
            static_assert(std::is_base_of_v<VaryingsBase, varyings_t>, "varyings_t 必须继承自 RGS::VaryingsBase");

            /* Vertex Shading & Projection */
            varyings_t varyings[RGS_MAX_VARYINGS];
            for (int i = 0; i < 3; i++)
            {
                program->VertexShader(varyings[i], triangle[i], *uniforms);
            }

            /* Clipping */
            int vertexNum = Clip(varyings);

            /* Screen Mapping */
            CaculateNdcPos(varyings, vertexNum);
            int fWidth = framebuffer.GetWidth();
            int fHeight = framebuffer.GetHeight();
            CaculateFragPos(varyings, vertexNum, (float)fWidth, (float)fHeight);

            /* Triangle Assembly & Rasterization */
            for (int i = 0; i < vertexNum - 2; i++)
            {
                varyings_t triVaryings[3];
                triVaryings[0] = varyings[0];
                triVaryings[1] = varyings[i + 1];
                triVaryings[2] = varyings[i + 2];

                RasterizeTriangle<vertex_t, uniforms_t, varyings_t, msaa>(framebuffer, *program, triVaryings, *uniforms);
            }
        }

        template<typename vertex_t, typename uniforms_t, typename varyings_t>
        static void Draw(Framebuffer& framebuffer,
                         std::shared_ptr<Program<vertex_t, uniforms_t, varyings_t>> program,
                         std::shared_ptr<Mesh<vertex_t>> mesh,
                         std::shared_ptr<uniforms_t> uniforms, 
                         const MSAA msaa)
        {
            // 在有管线的情况下想不到为什么会存在 Blend 时候不绘入 Depth 的情况 
            if (program->EnableBlend)
                program->EnableWriteDepth = true;

            std::function<void(Framebuffer&, std::shared_ptr<Program<vertex_t, uniforms_t, varyings_t>>, const Triangle<vertex_t>&, std::shared_ptr<uniforms_t>)> drawFun;

            switch (msaa)
            {
            case MSAA::None:
                drawFun = std::bind(DrawTriangle<vertex_t, uniforms_t, varyings_t, MSAA::None>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                break;
            case MSAA::X2:
                drawFun = std::bind(DrawTriangle<vertex_t, uniforms_t, varyings_t, MSAA::X2>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                break;
            case MSAA::X3:
                drawFun = std::bind(DrawTriangle<vertex_t, uniforms_t, varyings_t, MSAA::X3>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                break;
            case MSAA::X4:
                drawFun = std::bind(DrawTriangle<vertex_t, uniforms_t, varyings_t, MSAA::X4>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                break;
            case MSAA::X5:
                drawFun = std::bind(DrawTriangle<vertex_t, uniforms_t, varyings_t, MSAA::X5>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                break;
            case MSAA::X6:
                drawFun = std::bind(DrawTriangle<vertex_t, uniforms_t, varyings_t, MSAA::X6>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                break;
            case MSAA::X7:
                drawFun = std::bind(DrawTriangle<vertex_t, uniforms_t, varyings_t, MSAA::X7>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                break;
            case MSAA::X8:
                drawFun = std::bind(DrawTriangle<vertex_t, uniforms_t, varyings_t, MSAA::X8>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                break;
            default:
                ASSERT(false);
                return;
            }

            for (auto it = mesh->Begin(); it < mesh->End(); it++)
            {
                drawFun(framebuffer, program, *it, uniforms);
            }
        }
    };

}
