#pragma once
#include "Layer.h"

#include "RGS/Texture.h"
#include "RGS/Shader/IBLPBRShader.h"
#include "RGS/Shader/FlatColorShader.h"
#include "RGS/Render/Framebuffer.h"
#include "RGS/Render/Pipeline.h"

#include <string>
#include <memory>

namespace RGS {

    class IBLPBRLayer : public Layer
    { 
    public:
        IBLPBRLayer(std::string name)
            : Layer(name){}
        virtual ~IBLPBRLayer() override;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(float t) override;
        virtual void OnImGuiRender(float t) override;

    private:
        Texture* m_BrdfLUT;

        TextureSphere* m_SkyboxTex = nullptr;
        std::string m_SkyboxPath;

        LodTextureSphere* m_PrefilterEnvMap;
        std::string m_PrefilterEnvMapDir;

        TextureSphere* m_IrradianceMap;
        std::string m_IrradianceMapPath;

        std::shared_ptr<IBLPBRUniforms> m_IBLPBRUniforms;
        std::shared_ptr<FlatColorUniforms> m_FlatColorUniforms;

        bool m_DrawQuad = false;
        bool m_Running = true;
        int m_SkyboxTexIndex = 0;

        void RenderSkybox(Framebuffer& framebuffer, TextureSphere* skyboxTex, LodTextureSphere* lodSkyboxTex = nullptr, float roughness = 0.0f);
        void RenderSphere(Framebuffer& framebuffer);
        void RenderQuad(Framebuffer& framebuffer, const Vec3 pos = Vec3{0.0f, 0.0f, 0.0f}, const float sx = 1.0f, const float sy = 1.0f, const float rx = 0.0f, const float ry = 0.0f, const float rz = 0.0f);

        void OnValidate();

        void Dequote(std::string& str);

        Pipeline m_Pipeline;

        std::unique_ptr<Framebuffer> m_Framebuffer;
    };
}
