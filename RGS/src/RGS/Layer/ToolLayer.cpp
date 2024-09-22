#include "rgspch.h"
#include "ToolLayer.h"

#include "RGS/Render/Framebuffer.h"
#include "RGS/Render/Renderer.h"
#include "RGS/Texture.h"
#include "RGS/Timer.h"
#include "RGS/Config.h"
#include "RGS/Shader/BRDFShader.h"
#include "RGS/Shader/ConvSkyShader.h"

#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_image_resize2.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <thread>
#include <windows.h>
#ifdef _WIN32
    #include <direct.h> // For _mkdir on Windows
#else
    #include <sys/stat.h> // For mkdir on POSIX systems
#endif

namespace RGS {

    void ToolLayer::OnImGuiRender(float t)
    {
        ImGui::Begin(m_DebugName.c_str());

        ImGui::InputText("SkyboxPath", &m_SkyboxPath);
        ImGui::InputText("BRDFLutSaveDir", &m_BRDFLutSaveDir);
        ImGui::InputText("m_ConvDiffuseSaveDir", &m_ConvDiffuseSaveDir);

        // 去除前后双引号
        if (m_SkyboxPath[0] == '"')
            m_SkyboxPath.erase(0, 1);
        if (m_SkyboxPath[m_SkyboxPath.size() - 1] == '"')
            m_SkyboxPath.erase(m_SkyboxPath.size() - 1, m_SkyboxPath.size());

        /* Convolute */
        if (ImGui::Button(m_ConvoluteDiffuseButtonName.c_str()) && !m_Convoluting)
        {
            m_Convoluting = true;
            m_ConvoluteDiffuseButtonName = "Convoluting...";
            std::thread work{ [&]() {
                ConvoluteDiffuse(m_SkyboxPath, m_ConvDiffuseSaveDir);
                m_Convoluting = false;
                m_ConvoluteDiffuseButtonName = "ConvoluteDiffuse";
            } };
            work.detach();
        }

        /* Prefilter */
        if (ImGui::Button(m_PrefilterEnvMapButtonName.c_str()) && !m_Prefiltering)
        {
            m_Prefiltering = true;
            m_PrefilterEnvMapButtonName = "Prefiltering...";
            std::thread work{ [&]() {
                PrefilterEnvMap(m_SkyboxPath);
                m_Prefiltering = false;
                m_PrefilterEnvMapButtonName = "PrefilterEnvMap";
            }};
            work.detach();
        }
        
        /* Integrate */
        if (ImGui::Button(m_IntegrateBRDFButtonName.c_str()) && !m_Integrating)
        {
            m_Integrating = true;
            m_IntegrateBRDFButtonName = "Integrating...";
;
            std::thread work{ [&]() {
                IntegrateBRDF(m_BRDFLutSaveDir);
                m_Integrating = false;
                m_IntegrateBRDFButtonName = "IntegrateBRDF";
            }};
            work.detach();
        }

        ImGui::End();
    }

    void ToolLayer::ConvoluteDiffuse(std::string skyboxPath, std::string saveDir)
    {
        if (auto index = skyboxPath.rfind(".hdr");
            index == std::string::npos || index + 4 != skyboxPath.size())
        {
            std::cout << "非法后缀" << std::endl;
            return;
        }
        auto index = skyboxPath.rfind("\\");
        if (index == std::string::npos)
        {
            std::cout << "加载失败" << std::endl;
            return;
        }

        TextureSphere* tex = TextureSphere::LoadTextureSphere(skyboxPath);
        if (tex == nullptr)
        {
            std::cout << "加载失败" << std::endl;
            delete tex;
            return;
        }

        Timer timer;

        std::string savePath = skyboxPath.substr(index + 1, skyboxPath.size() - index - 5);
        savePath += ".hdr";
        savePath.insert(0, saveDir + "\\conv_diffuse_");

        int width = Config::ConvDiffuseWidth;
        int height = width / 2;

        auto framebuffer = Framebuffer::Create(width, height);


        auto program = std::make_shared<Program<ConvSkyVertex, ConvSkyUniforms, ConvSkyVaryings>>(ConvSkyVertexShader, ConvSkyFragmentShader);
        program->EnableJobSystem = false;

        auto uniforms = std::make_shared<ConvSkyUniforms> ();
        uniforms->MVP = Mat4Perspective(90.0f / 360.0f * 2.0f * PI, 1, 0.1f, 100.0f) * Mat4Identity();
        uniforms->SkyboxTex = tex;


        Triangle<ConvSkyVertex> tri;
        tri.Vertex[0].ModelPos = { -5, -5, -5, 1 };
        tri.Vertex[1].ModelPos = { 5, -5, -5, 1 };
        tri.Vertex[2].ModelPos = { -5, 5, -5, 1 };
        Renderer::DrawTriangle(*framebuffer, program, tri, uniforms);

        tri.Vertex[0].ModelPos = { 5, -5, -5, 1 };
        tri.Vertex[1].ModelPos = { 5, 5, -5, 1 };
        tri.Vertex[2].ModelPos = { -5, 5, -5, 1 };
        Renderer::DrawTriangle(*framebuffer, program, tri, uniforms);

        stbi_flip_vertically_on_write(true);
        stbi_write_hdr(savePath.c_str(), width, height, 3, framebuffer->GetRawColorData());
        delete tex;

        double duration = timer.GetDuration();
        std::cout << "Complete ConvDiffuse in " << duration / 1000. << "s" << std::endl;
    }

    void ToolLayer::PrefilterEnvMap(std::string skyboxPath)
    {
        if (auto index = skyboxPath.rfind(".hdr");
            index == std::string::npos || index + 4 != skyboxPath.size())
        {
            std::cout << "非法后缀" << std::endl;
            return;
        }
        auto index = skyboxPath.rfind("\\");
        if (index == std::string::npos)
        {
            std::cout << "加载失败" << std::endl;
            return;
        }

        std::string prefilterEnvMapDir = skyboxPath.substr(index + 1, skyboxPath.size() - index - 5);
        prefilterEnvMapDir.insert(0, ".\\Assets\\prefilter_envmap\\");

#ifdef _WIN32
    int ret = _mkdir(prefilterEnvMapDir.c_str());
#else
    mkdir(prefilterEnvMapDir.c_str());
#endif
       
        if (ret == -1)
        {
            std::cout << "创建文件夹失败" << std::endl;
            return;
        }

        std::unique_ptr<LodTextureSphere> skybox(new LodTextureSphere(skyboxPath));
        
        for (int i = 4; i >= 0; --i)
        {
            float roughness = 0.25f * (float)i;
            int width = Config::PrefilterEnvMapMinWidth * (5 - i);
            int height = width / 2;

            Timer timer;

            auto framebuffer = Framebuffer::Create(width, height);

            auto program = std::make_shared<Program<PrefilterVertex, PrefilterUniforms, PrefilterVaryings>>(PrefilterVertexShader, PrefilterFragmentShader);
            program->EnableJobSystem = false;

            auto uniforms = std::make_shared<PrefilterUniforms>() ;
            uniforms->MVP = Mat4Perspective(90.0f / 360.0f * 2.0f * PI, 1, 0.1f, 100.0f) * Mat4Identity();
            uniforms->SkyboxTex = skybox.get();
            uniforms->Roughness = roughness;

            Triangle<ConvSkyVertex> tri;
            tri.Vertex[0].ModelPos = { -5, -5, -5, 1 };
            tri.Vertex[1].ModelPos = { 5, -5, -5, 1 };
            tri.Vertex[2].ModelPos = { -5, 5, -5, 1 };
            Renderer::DrawTriangle(*framebuffer, program, tri, uniforms);

            tri.Vertex[0].ModelPos = { 5, -5, -5, 1 };
            tri.Vertex[1].ModelPos = { 5, 5, -5, 1 };
            tri.Vertex[2].ModelPos = { -5, 5, -5, 1 };
            Renderer::DrawTriangle(*framebuffer, program, tri, uniforms);

            stbi_flip_vertically_on_write(true);
            std::string path{ prefilterEnvMapDir };
            path.append("\\");
            path.append(std::to_string(i));
            path.append(".hdr");
            stbi_write_hdr(path.c_str(), width, height, 3, framebuffer->GetRawColorData());

            m_Prefiltering = false;
            m_PrefilterEnvMapButtonName = "Prefilter";
            double duration = timer.GetDuration();
            std::cout << "Complete PrefilterEnvMap in " << duration / 1000. << "s" << std::endl;
            std::cout << " - Path: " << path << std::endl;
        }
    }

    void ToolLayer::IntegrateBRDF(std::string saveDir)
    {
        std::string path = saveDir + "\\brdf.jpg";
        int width = Config::IntegrateBRDFWidth;
        int height = width;
        std::unique_ptr<unsigned char[]> data(new unsigned char[width * height * 3]);
        int ret = stbi_write_jpg(path.c_str(), width, height, 3, data.get(), 100);
        if (ret == 0)
        {
            std::cout << "写入失败" << std::endl;
            return;
        }

        Timer timer;

        auto framebuffer = Framebuffer::Create(width, height);
        auto program = std::make_shared<Program<BRDFVertex, BRDFUniforms, BRDFVaryings>>(BRDFVertexShader, BRDFFragmentShader);
        program->EnableJobSystem = false;
        auto uniforms = std::make_shared<BRDFUniforms>();
        uniforms->MVP = Mat4Perspective(90.0f / 360.0f * 2.0f * PI, 1, 0.1f, 100.0f) * Mat4Identity();
        
        Triangle<BRDFVertex> tri;
        tri.Vertex[0].ModelPos = { -5, -5, -5, 1 };
        tri.Vertex[1].ModelPos = { 5, -5, -5, 1 };
        tri.Vertex[2].ModelPos = { -5, 5, -5, 1 };
        Renderer::DrawTriangle(*framebuffer, program, tri, uniforms);
        tri.Vertex[0].ModelPos = { 5, -5, -5, 1 };
        tri.Vertex[1].ModelPos = { 5, 5, -5, 1 };
        tri.Vertex[2].ModelPos = { -5, 5, -5, 1 };
        Renderer::DrawTriangle(*framebuffer, program, tri, uniforms);
        
        auto buf = framebuffer->GetRGBColorData();
        stbi_flip_vertically_on_write(true);
        ret = stbi_write_jpg(path.c_str(), width, height, 3, buf.get(), 100);
        if (ret == 0)
        {
            std::cout << "写入失败" << std::endl;
        }
        else
        {
            double duration = timer.GetDuration();
            std::cout << "Complete IntegrateBRDF in " << duration / 1000. << " s" << std::endl;
        }
    }
}