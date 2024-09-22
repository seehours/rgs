#include "rgspch.h"
#include "Model.h"
#include "RGS/Shader/BlinnShader.h"

#include <fstream>
#include <stdio.h>
#include <iomanip> 

namespace RGS {

    void Mesh::LoadMesh(const char* fileName)
    {
        std::ifstream file(fileName);
        ASSERT(file);

        std::string line;
        while (!file.eof())
        {
            std::getline(file, line);
            int items = -1;
            if (line.find("v ") == 0)                /* Position */
            {
                Vec3 position;
                items = sscanf(line.c_str(), "v %f %f %f",
                               &position.X, &position.Y, &position.Z);
                ASSERT(items == 3);
                Positions.push_back(position);
            }
            else if (line.find("vt ") == 0)          /* Texcoord */
            {
                Vec2 texcoord;
                items = sscanf(line.c_str(), "vt %f %f",
                               &texcoord.X, &texcoord.Y);
                ASSERT(items == 2);
                TexCoords.push_back(texcoord);
            }
            else if (line.find("vn ") == 0)          /* Normal */
            {             
                Vec3 normal;
                items = sscanf(line.c_str(), "vn %f %f %f",
                               &normal.X, &normal.Y, &normal.Z);
                ASSERT(items == 3);
                Normals.push_back(normal);
            }
            else if (line.find("f ") == 0)           /* Face */
            {              
                 int posIndices[3], uvIndices[3], nIndices[3];
                 items = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                                &posIndices[0], &uvIndices[0], &nIndices[0],
                                &posIndices[1], &uvIndices[1], &nIndices[1],
                                &posIndices[2], &uvIndices[2], &nIndices[2]);
                 ASSERT(items == 9);
                 for (int i = 0; i < 3; i++) 
                 {
                     PosIndices.push_back(posIndices[i] - 1);
                     TexIndices.push_back(uvIndices[i] - 1);
                     NormalIndices.push_back(nIndices[i] - 1);
                 }

            }
        }
        file.close();
    }

    int Mesh::GetFaceNum() const
    {
        return (int)PosIndices.size() / 3;
    }

    BlinnModel::BlinnModel(const char* fileName)
        :m_Program(BlinnVertexShader, BlinnFragmentShader)
    {
        m_Mesh.LoadMesh(fileName);
    }

    void BlinnModel::Draw(Framebuffer& framebuffer)
    {
        Triangle<BlinnVertex> triangle;
        for (int i = 0; i < m_Mesh.GetFaceNum(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                int index = 3 * i + j;
                int posIndex = m_Mesh.PosIndices[index];
                int texIndex = m_Mesh.TexIndices[index];
                int nIndex = m_Mesh.NormalIndices[index];
                triangle[j].ModelPos = { m_Mesh.Positions[posIndex] , 1.0f};
                triangle[j].TexCoord = m_Mesh.TexCoords[texIndex];
                triangle[j].ModelNormal = m_Mesh.Normals[nIndex];
            }
            Renderer::Draw(framebuffer, m_Program, triangle, m_Uniforms);
        }
    }

}