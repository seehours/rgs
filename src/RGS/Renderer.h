#pragma once 
#include "RGS/Framebuffer.h"
#include "RGS/Base.h"
#include "RGS/Maths.h"

#include <type_traits>
#include <cmath>

namespace RGS {

    template<typename vertex_t>
    struct Triangle
    {
        static_assert(std::is_base_of_v<VertexBase, vertex_t>, "vertex_t 必须继承自 RGS::VertexBase");

        vertex_t Vertex[3];

        vertex_t& operator[](size_t i) { return Vertex[i]; }
        const vertex_t& operator[](size_t i) const { return Vertex[i]; }

        Triangle() = default;
    };

    template<typename vertex_t, typename uniforms_t, typename varyings_t>
    struct Program
    {
        using vertex_shader_t = void (*)(varyings_t&, const vertex_t&, const uniforms_t&);
        vertex_shader_t VertexShader;

        Program(const vertex_shader_t vertexShader)
            : VertexShader(vertexShader) {}
    };

    class Renderer
    {
    private:
        static constexpr int RGS_MAX_VARYINGS = 9;

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

        template <typename varyings_t>
        static void LerpVaryings(varyings_t& out, const varyings_t& start, const varyings_t& end, const float ratio)
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

        static bool IsVertexVisible(const Vec4& clipPos)
        {
            return (std::fabs(clipPos.X) <= clipPos.W 
                 && std::fabs(clipPos.Y) <= clipPos.W 
                 && std::fabs(clipPos.Z) <= clipPos.W);
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
                return (prev.W - prev.X) / ((prev.W - prev.X) - (curr.W - curr.X));
            case Plane::NEGATIVE_X:
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
                int prevIndex = (inVertexNum - 1 + i) % inVertexNum;
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

    public:

        template<typename vertex_t, typename uniforms_t, typename varyings_t>
        static void Draw(Framebuffer& framebuffer, 
                         const Program<vertex_t, uniforms_t, varyings_t>& program,
                         const Triangle<vertex_t>& triangle, 
                         const uniforms_t& uniforms)
        {
            static_assert(std::is_base_of_v<VertexBase, vertex_t>, "vertex_t 必须继承自 RGS::VertexBase");
            static_assert(std::is_base_of_v<VaryingsBase, varyings_t>, "varyings_t 必须继承自 RGS::VaryingsBase");

            /* Vertex Shading & Projection */
            varyings_t varyings[RGS_MAX_VARYINGS];
            for (int i = 0; i < 3; i++)
            {
                program.VertexShader(varyings[i], triangle[i], uniforms);
            }

            /* Clipping */
            int vertexNum = Clip(varyings);
        }
    };
}