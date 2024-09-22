#include "rgspch.h"
#include "IBLPBRLayer.h"

#include "Application.h"
#include "CameraLayer.h"
#include "RGS/Texture.h"
#include "RGS/Render/Framebuffer.h"
#include "RGS/Render/Renderer.h"
#include "RGS/Render/Mesh.h"
#include "RGS/Render/Pipeline.h"
#include "RGS/Render/RenderCommand.h"

#include "RGS/Shader/SkyboxShader.h"
#include "RGS/Shader/IBLPBRShader.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <stb_image_write.h>

namespace RGS {

    IBLPBRLayer::~IBLPBRLayer()
    {
        if (m_IrradianceMap)
            delete m_IrradianceMap;
        if (m_PrefilterEnvMap)
            delete m_PrefilterEnvMap;
        if (m_BrdfLUT)
            delete m_BrdfLUT;
        if (m_SkyboxTex)
            delete m_SkyboxTex;

        m_IrradianceMap = nullptr;
        m_PrefilterEnvMap = nullptr;
        m_BrdfLUT = nullptr;
        m_SkyboxTex = nullptr;
    }

    void IBLPBRLayer::OnAttach()
    {
        m_IrradianceMap = new TextureSphere("Assets\\diffuse_conv.hdr");
        m_PrefilterEnvMap = new LodTextureSphere({ "Assets\\prefilter\\prefilter-800x400-0.00.hdr",
                                                   "Assets\\prefilter\\prefilter-640x320-0.25.hdr",
                                                   "Assets\\prefilter\\prefilter-480x240-0.50.hdr",
                                                   "Assets\\prefilter\\prefilter-320x160-0.75.hdr",
                                                   "Assets\\prefilter\\prefilter-160x80-1.00.hdr" });
        m_BrdfLUT = new Texture("Assets\\brdf.jpg");
        m_SkyboxTex = new TextureSphere("Assets\\hdr\\newport_loft.hdr");
        m_SkyboxPath = m_SkyboxTex->GetPath();
        m_IrradianceMapPath = m_IrradianceMap->GetPath();
        m_PrefilterEnvMapDir = m_PrefilterEnvMap->GetPath();

        m_IBLPBRUniforms = std::make_shared<IBLPBRUniforms>();
        m_IBLPBRUniforms->Albedo = { 1.0f, 1.0f, 1.0f };
        m_IBLPBRUniforms->Ao = 1.0f;
        m_IBLPBRUniforms->Metallic = 0.85f;
        m_IBLPBRUniforms->Roughness = 0.275f;

        m_IBLPBRUniforms->BrdfLUT = m_BrdfLUT;
        m_IBLPBRUniforms->IrradianceMap = m_IrradianceMap;
        m_IBLPBRUniforms->PrefilterMap = m_PrefilterEnvMap;

        m_IBLPBRUniforms->LightPos = { 10.0f, 10.0f, 10.0f };
        m_IBLPBRUniforms->LightColor = { 1000.0f, 500.0f, 400.0f };

        m_FlatColorUniforms = std::make_shared<FlatColorUniforms>();

        m_FlatColorUniforms->Color = { 0.1f, 1.0f, 1.0f, 0.5f };

        auto& framebuffer = Application::Instance().GetFramebuffer();
        m_Framebuffer = Framebuffer::Create(framebuffer.GetWidth(), framebuffer.GetHeight());
    }

    void IBLPBRLayer::OnDetach()
    {
        if (m_IrradianceMap)
            delete m_IrradianceMap;
        if (m_PrefilterEnvMap)
            delete m_PrefilterEnvMap;
        if (m_BrdfLUT)
            delete m_BrdfLUT;
        if (m_SkyboxTex)
            delete m_SkyboxTex;

        m_IrradianceMap = nullptr;
        m_PrefilterEnvMap = nullptr;
        m_BrdfLUT = nullptr;
        m_SkyboxTex = nullptr;
    }

    void IBLPBRLayer::OnUpdate(float t)
    {
        if (!m_Running)
            return;

        m_Pipeline.BeginFrame();

        m_Pipeline.AddCommand(RenderCommand::Clear(*m_Framebuffer), RenderStage::BeginFrame);
        m_Pipeline.AddCommand(RenderCommand::ClearDepth(*m_Framebuffer), RenderStage::BeginFrame);

        const Camera& camera = CameraLayer::Get().GetCamera();
        Mat4 view = camera.ViewMat4();

        Mat4 normalToWorld = Mat4Identity();
        normalToWorld.M[0][3] = 0.0f;
        normalToWorld.M[1][3] = 0.0f;
        normalToWorld.M[2][3] = 0.0f;

        m_IBLPBRUniforms->CamPos = camera.Pos;
        m_IBLPBRUniforms->ModelMatrix = Mat4Identity();
        m_IBLPBRUniforms->MVP = camera.ProjectionMat4() * view;
        m_IBLPBRUniforms->NormalMatrix = normalToWorld;
        RenderSphere(*m_Framebuffer);

        // RenderSkybox
        switch (m_SkyboxTexIndex)
        {
        case 0:
            RenderSkybox(*m_Framebuffer, m_SkyboxTex);
            break;
        case 1:
            RenderSkybox(*m_Framebuffer, m_IrradianceMap);
            break;
        case 2:
            RenderSkybox(*m_Framebuffer, nullptr, m_PrefilterEnvMap, m_IBLPBRUniforms->Roughness);
            break;
        default:
            break;
        }

        if (m_DrawQuad)
            RenderQuad(*m_Framebuffer, { 0.0f, 0.0f, 1.3f });

        m_Pipeline.AddCommand(RenderCommand::ResolveParallel(*m_Framebuffer, true), RenderStage::EndFrame);
        m_Pipeline.AddCommand(RenderCommand::BlitToScreen(*m_Framebuffer), RenderStage::EndFrame);

        m_Pipeline.EndFrame();

    }

    void IBLPBRLayer::OnImGuiRender(float t)
    {
        ImGui::Begin(m_DebugName.c_str());
        ImGui::Checkbox("Running", &m_Running);
        if (m_Running)
        {
            ImGui::Checkbox("Draw Quad", &m_DrawQuad);

            static int msaaLevel = 3;
            ImGui::DragInt("MSAA Level", &msaaLevel, 0.05f, 1, 8);
            if (msaaLevel != (int)m_Framebuffer->GetMSAA())
            {   
                uint32_t width = m_Framebuffer->GetWidth();
                uint32_t height = m_Framebuffer->GetHeight();
                m_Framebuffer = Framebuffer::Create(width, height, (MSAA)msaaLevel);
            }   

            ImGui::Text("Skybox Tex: ");
            ImGui::RadioButton("Skybox", &m_SkyboxTexIndex, 0); ImGui::SameLine();
            ImGui::RadioButton("Irradiance", &m_SkyboxTexIndex, 1); ImGui::SameLine();
            ImGui::RadioButton("Prefilter", &m_SkyboxTexIndex, 2);

            constexpr float speed = 0.005f;
            ImGui::Spacing();
            ImGui::DragFloat3("Albedo", (float*)&m_IBLPBRUniforms->Albedo, speed, 0.0f, 1.0f);
            ImGui::DragFloat("Ao", &m_IBLPBRUniforms->Ao, speed, 0.0f, 1.0f);
            ImGui::DragFloat("Metallic", &m_IBLPBRUniforms->Metallic, speed, 0.0f, 1.0f);
            ImGui::DragFloat("Roughness", &m_IBLPBRUniforms->Roughness, speed, 0.0f, 1.0f);

            ImGui::Spacing();
            ImGui::DragFloat3("Light Pos", (float*)&m_IBLPBRUniforms->LightPos);
            ImGui::DragFloat3("Light Color", (float*)&m_IBLPBRUniforms->LightColor);
        }

        ImGui::InputText("SkyboxPath", &m_SkyboxPath);
        Dequote(m_SkyboxPath);

        ImGui::InputText("IrradianceMapPath", &m_IrradianceMapPath);
        Dequote(m_IrradianceMapPath);

        ImGui::InputText("PrefilterEnvMapDir", &m_PrefilterEnvMapDir);

        if (ImGui::Button("Apply"))
            OnValidate();
        ImGui::End();
    }

    void IBLPBRLayer::RenderSkybox(Framebuffer& framebuffer, TextureSphere* skyboxTex, LodTextureSphere* lodSkyboxTex, float roughness)
    {
        static bool firstLoop = true;
        static std::shared_ptr<Mesh<IBLPBRVertex>> boxMesh;
        static std::shared_ptr<Program<SkyboxVertex, SkyboxUniforms, SkyboxVaryings>> program;
        if (firstLoop)
        {
            boxMesh = Mesh<IBLPBRVertex>::CreateBoxMesh();
            program = std::make_shared<Program<SkyboxVertex, SkyboxUniforms, SkyboxVaryings>>(SkyboxVertexShader, SkyboxFragmentShader);
            program->DepthFunc = DepthFuncType::LEQUAL;
            program->EnableDoubleSided = true;
            firstLoop = false;
        }

        // Uniforms
        std::shared_ptr<SkyboxUniforms> uniforms = std::make_shared<SkyboxUniforms>();
        const Camera& camera = CameraLayer::Get().GetCamera();
        Mat4 view = camera.ViewMat4();
        view.M[0][3] = 0.0f;
        view.M[1][3] = 0.0f;
        view.M[2][3] = 0.0f;
        uniforms->MVP = camera.ProjectionMat4() * view;
        uniforms->SkyboxTex = skyboxTex;
        uniforms->LodSkyboxTex = lodSkyboxTex;
        uniforms->Lod = roughness;

        m_Pipeline.AddCommand(RenderCommand::Draw(framebuffer, program, boxMesh, uniforms, framebuffer.GetMSAA()), RenderStage::Geometry);
    }

    void IBLPBRLayer::RenderSphere(Framebuffer& framebuffer)
    {
        static bool firstLoop = true;
        static std::shared_ptr<Mesh<IBLPBRVertex>> sphereMesh;
        static std::shared_ptr<Program<IBLPBRVertex, IBLPBRUniforms, IBLPBRVaryings>> program;
        if (firstLoop)
        {
            sphereMesh = Mesh<IBLPBRVertex>::CreateSphereMesh();
            program = std::make_shared<Program<IBLPBRVertex, IBLPBRUniforms, IBLPBRVaryings>>(IBLPBRVertexShader, IBLPBRFragmentShader);
            firstLoop = false;
        }

        for (int i = 0; i < 1; ++i)
        {
            const Camera& camera = CameraLayer::Get().GetCamera();
            Mat4 view = camera.ViewMat4();
            Mat4 model = Mat4Translate(i * 2.5f, 0, 0);
            m_IBLPBRUniforms->ModelMatrix = model;
            m_IBLPBRUniforms->MVP = camera.ProjectionMat4() * view * model;

            Mat4 normalToWorld = model;
            normalToWorld.M[0][3] = 0.0f;
            normalToWorld.M[1][3] = 0.0f;
            normalToWorld.M[2][3] = 0.0f;
            m_IBLPBRUniforms->NormalMatrix = normalToWorld;

            m_Pipeline.AddCommand(RenderCommand::Draw(framebuffer, program, sphereMesh, m_IBLPBRUniforms, framebuffer.GetMSAA()), RenderStage::Geometry);
        }
    }

    void IBLPBRLayer::RenderQuad(Framebuffer& framebuffer, const Vec3 pos, const float sx, const float sy, const float rx, const float ry, const float rz)
    {
        m_FlatColorUniforms->Color = { 0.1, 1.0, 1.0, 0.5 };
        static bool firstLoop = true;
        static std::shared_ptr<Mesh<FlatColorVertex>> quadMesh;
        static std::shared_ptr<Program<FlatColorVertex, FlatColorUniforms, FlatColorVaryings>> program;
        if (firstLoop)
        {
            quadMesh = Mesh<FlatColorVertex>::CreateQuadMesh();
            program = std::make_shared<Program<FlatColorVertex, FlatColorUniforms, FlatColorVaryings>>(FlatColorVertexShader, FlatColorFragmentShader);
            program->EnableBlend = true;
            program->EnableWriteDepth = false;
            program->EnableDoubleSided = true;
            firstLoop = false;
        }
        Mat4 mvp = Mat4Translate(pos.X, pos.Y, pos.Z);
        mvp = Mat4Scale(sx, sy, 1.0f) * mvp;
        mvp = Mat4RotateZ(rz) * Mat4RotateY(ry) * Mat4RotateX(rx) * mvp;
        const Camera& camera = CameraLayer::Get().GetCamera();
        mvp = camera.ProjectionMat4() * camera.ViewMat4() * mvp;
        m_FlatColorUniforms->MVP = mvp;
        m_Pipeline.AddCommand(RenderCommand::Draw(framebuffer, program, quadMesh, m_FlatColorUniforms, framebuffer.GetMSAA()), RenderStage::Transparent);
    }

    void IBLPBRLayer::OnValidate()
    {
        if (m_SkyboxPath != m_SkyboxTex->GetPath())
        {
            TextureSphere* tex = TextureSphere::LoadTextureSphere(m_SkyboxPath);
            if (tex == nullptr)
            {
                m_SkyboxPath = m_SkyboxTex->GetPath();
                std::cout << "加载失败" << std::endl;
            }
            else
            {
                delete m_SkyboxTex;
                m_SkyboxTex = tex;
            }
        }

        if (m_IrradianceMapPath != m_IrradianceMap->GetPath())
        {
            TextureSphere* tex = TextureSphere::LoadTextureSphere(m_IrradianceMapPath);
            if (tex == nullptr)
            {
                m_IrradianceMapPath = m_IrradianceMap->GetPath();
                std::cout << "加载失败" << std::endl;
            }
            else
            {
                delete m_IrradianceMap;
                m_IrradianceMap = tex;
                m_IBLPBRUniforms->IrradianceMap = m_IrradianceMap;
            }
        }

        if (m_PrefilterEnvMapDir != m_PrefilterEnvMap->GetPath())
        {
            LodTextureSphere* tex = LodTextureSphere::LoadLodTextureSphere(m_PrefilterEnvMapDir, LodTextureSphere::LoadType::Directory);
            if (tex == nullptr)
            {
                m_PrefilterEnvMapDir = m_PrefilterEnvMap->GetPath();
                std::cout << "加载失败" << std::endl;
            }
            else
            {
                delete m_PrefilterEnvMap;
                m_PrefilterEnvMap = tex;
                m_IBLPBRUniforms->PrefilterMap = m_PrefilterEnvMap;
            }
        }
    }

    void IBLPBRLayer::Dequote(std::string& str)
    {
        if (str[0] == '"')
            str.erase(0, 1);
        if (str[str.size() - 1] == '"')
            str.erase(str.size() - 1, str.size());
    }
}
