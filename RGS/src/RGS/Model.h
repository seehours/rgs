#pragma once
#include "RGS/Maths.h"
#include "RGS/Framebuffer.h"
#include "RGS/Renderer.h"

#include "RGS/Shader/BlinnShader.h"
#include <vector>

namespace RGS {

    class Mesh
    {
    public:
        std::vector<Vec3> Positions;
        std::vector<Vec2> TexCoords;
        std::vector<Vec3> Normals;
        std::vector<int> PosIndices;
        std::vector<int> TexIndices;
        std::vector<int> NormalIndices;

    public:
        void LoadMesh(const char* fileName);
        int GetFaceNum() const;
    };

    class BlinnModel
    {
        using program_t = Program<BlinnVertex, BlinnUniforms, BlinnVaryings>;
    public:
        BlinnModel(const char *fileName);
        
        void Draw(Framebuffer& framebuffer);

        void SetUniforms(const BlinnUniforms& uniforms) { m_Uniforms = uniforms; }
        void SetProgram(const program_t& program) { m_Program = program; }

    private:
        Mesh m_Mesh;
        program_t m_Program;
        BlinnUniforms m_Uniforms;
    };

}