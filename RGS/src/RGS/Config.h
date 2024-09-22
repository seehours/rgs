#pragma once

#define RGS_PROFILE 0

#define RGS_ENABLE_WIREFRAME_MODE 0

namespace RGS {
	namespace Config
	{
		constexpr int PrefilterEnvMapMinWidth = 256;
		constexpr int IntegrateBRDFWidth = 512;
		constexpr int LodTextureSphereMaxWidth = 2048;
		constexpr int ConvDiffuseWidth = 256; 

		// -----------------------------
		//          Job System
		// -----------------------------
		constexpr bool LimitToSingleThread = false;

	};
}