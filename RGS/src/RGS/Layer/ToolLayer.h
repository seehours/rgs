#pragma once 
#include "Layer.h"
#include "RGS/Texture.h"
#include <string>

namespace RGS {

	class ToolLayer : public Layer
	{
	public:
		ToolLayer(std::string name)
			: Layer(name) {}
		~ToolLayer() {}

		virtual void OnAttach() override {}
		virtual void OnDetach()  override {}
		virtual void OnUpdate(float t) override {}
		virtual void OnImGuiRender(float t) override;

	private:
		// Convolute Diffuse
		std::string m_ConvoluteDiffuseButtonName = "ConvoluteDiffuse";
		bool m_Convoluting = false;
		std::string m_SkyboxPath = ".\\Assets\\hdr\\container_free_hdr\\Container_Free\\container_free_Ref.hdr";
		std::string m_ConvDiffuseSaveDir = ".\\Assets";

		// Prefilter EnvMap
		std::string m_PrefilterEnvMapButtonName = "PrefilterEnvMap";
		bool m_Prefiltering = false;

		// Integrate BRDF
		std::string m_IntegrateBRDFButtonName = "IntegrateBRDF";
		bool m_Integrating = false;
		std::string m_BRDFLutSaveDir = ".\\Assets";

		void ConvoluteDiffuse(std::string skyboxPath, std::string saveDir);
		void PrefilterEnvMap(std::string skyboxPath);
		void IntegrateBRDF(std::string saveDir);
	};

}