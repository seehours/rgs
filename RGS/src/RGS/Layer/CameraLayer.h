#pragma once 
#include "Layer.h"

#include "RGS/Base/Maths.h"
#include "RGS/Render/Renderer.h"

#include <string>

namespace RGS {

    class CameraLayer : public Layer
    {
    public:
        CameraLayer(std::string name)
            : Layer(name) 
        {
            ASSERT(!s_Instance);
            s_Instance = this;
        }

        virtual void OnAttach() override;
        virtual void OnDetach()  override;
        virtual void OnUpdate(float t) override;
        virtual void OnImGuiRender(float t) override;

        const Camera& GetCamera() const { return m_Camera; }

        static CameraLayer& Get() { return *s_Instance; }
    private:
        Vec4 m_BackgroundColor = { 0.0f, 0.0f, 0.0f, 0.0f };
        Camera m_Camera;

        static CameraLayer* s_Instance;
    };

}