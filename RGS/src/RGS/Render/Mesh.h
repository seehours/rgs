#pragma once
#include "RGS/Shader/ShaderBase.h"

#include <vector>
#include <memory>
#include <iterator>

namespace RGS {

    template<typename vertex_t>
    struct Triangle
    {
        static_assert(std::is_base_of_v<VertexBase, vertex_t>, "Triangle 的模板参数必须继承自 RGS::VertexBase");

        vertex_t Vertex[3];

        vertex_t& operator[](size_t i) { return Vertex[i]; }
        const vertex_t& operator[](size_t i) const { return Vertex[i]; }

        Triangle() = default;
    };

    template<typename vertex_t>
    struct Mesh
    {
        std::vector<Triangle<vertex_t>> Triangles;

        inline typename std::vector<Triangle<vertex_t>>::iterator Begin()
        {
            return Triangles.begin();
        }

        inline typename std::vector<Triangle<vertex_t>>::iterator End() 
        {  
            return Triangles.end();    
        }

        static std::shared_ptr<Mesh<VertexBase3D>> CreateSphereMesh();
        static std::shared_ptr<Mesh<VertexBase3D>> CreateBoxMesh();
        static std::shared_ptr<Mesh<VertexBase3D>> CreateQuadMesh();
    };

    template<typename vertex_t>
    std::shared_ptr<Mesh<VertexBase3D>> Mesh<vertex_t>::CreateSphereMesh()
    {
        auto mesh = std::make_shared<Mesh<VertexBase3D>>();

        std::vector<Vec3> positions;
        std::vector<Vec2> uv;
        std::vector<Vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;

        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(Vec3(xPos, yPos, zPos));
                uv.push_back(Vec2(xSegment, ySegment));
                normals.push_back(Vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }

        for (unsigned int i = 2; i < indices.size(); ++i)
        {
            Triangle<VertexBase3D> tri;

            uint32_t index0;
            uint32_t index1;
            uint32_t index2;
            if (i % 2 == 1)
            {
                index0 = indices[i - 1];
                index1 = indices[i - 2];
                index2 = indices[i];
            }
            else
            {
                index0 = indices[i - 2];
                index1 = indices[i - 1];
                index2 = indices[i];
            }

            tri.Vertex[0].ModelPos = { positions[index0], 1.0f };
            tri.Vertex[1].ModelPos = { positions[index1], 1.0f };
            tri.Vertex[2].ModelPos = { positions[index2], 1.0f };

            if (normals.size() > 0)
            {
                tri.Vertex[0].ModelNormal = normals[index0];
                tri.Vertex[1].ModelNormal = normals[index1];
                tri.Vertex[2].ModelNormal = normals[index2];
            }
            if (uv.size() > 0)
            {
                /*tri.Vertex[0].TexCoord = uv[index0];
                tri.Vertex[1].TexCoord = uv[index1];
                tri.Vertex[2].TexCoord = uv[index2];*/
            }
            mesh->Triangles.emplace_back(tri);
        }

        return mesh;
    }

    template<typename vertex_t>
    std::shared_ptr<Mesh<VertexBase3D>> Mesh<vertex_t>::CreateBoxMesh()
    {
        auto mesh = std::make_shared<Mesh<VertexBase3D>>();

        float boxVertices[] = 
        {
            // positions         normal
            // -Z 
            -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
             1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
             1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
             1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
            -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f,

            // -X 
            -1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f, -1.0f, 0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f, -1.0f, 0.0f, 0.0f,

            // +X 
             1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,

             // +Z 
             -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
             -1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
              1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
              1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
              1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
             -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,

             // +Y
             -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
              1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
              1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
              1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
             -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
             -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

             // -Y
             -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
             -1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f,
              1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
              1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
             -1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f,
              1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f
        };

        constexpr uint32_t floatNumofTriangle = sizeof(boxVertices) / sizeof(float) / 12;
        constexpr uint32_t floatNumofVeterx = floatNumofTriangle / 3;
        for (uint32_t i = 0; i < 12; i++)
        {
            Triangle<VertexBase3D> tri;
            for (uint32_t j = 0; j < 3; j++)
            {
                uint32_t offset = floatNumofTriangle * i + floatNumofVeterx * j;

                tri.Vertex[j].ModelPos = { boxVertices[offset + 0], boxVertices[offset + 1] ,boxVertices[offset + 2], 1.0f };
                tri.Vertex[j].ModelNormal = { boxVertices[offset + 3], boxVertices[offset + 4] ,boxVertices[offset + 5] };
            }
            mesh->Triangles.emplace_back(tri);
        }
        return mesh;
    }

    template<typename vertex_t>
    std::shared_ptr<Mesh<VertexBase3D>> Mesh<vertex_t>::CreateQuadMesh()
    {
        auto mesh = std::make_shared<Mesh<vertex_t>>();

        // Define vertices for a unit quad
        float quadVertices[] =
        {
            // positions         // normals
            -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  // Top-left
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  // Bottom-left
             0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  // Bottom-right
            
             0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  // Bottom-right
             0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  // Top-right
            -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f   // Top-left
        };

        constexpr uint32_t numVerticesPerTriangle = 3;
        constexpr uint32_t numTriangles = 2; // 2 triangles
        constexpr uint32_t numVertices = numTriangles * numVerticesPerTriangle; // Total vertices

        for (uint32_t i = 0; i < numTriangles; ++i)
        {
            Triangle<vertex_t> tri;
            for (uint32_t j = 0; j < numVerticesPerTriangle; ++j)
            {
                uint32_t offset = (i * numVerticesPerTriangle + j) * 6; // 6 floats per vertex (position + normal)

                tri.Vertex[j].ModelPos = { quadVertices[offset + 0], quadVertices[offset + 1], quadVertices[offset + 2], 1.0f };
                tri.Vertex[j].ModelNormal = { quadVertices[offset + 3], quadVertices[offset + 4], quadVertices[offset + 5] };
            }
            mesh->Triangles.emplace_back(tri);
        }

        return mesh;
    }
}
