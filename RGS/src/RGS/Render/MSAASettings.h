#pragma once
#include "RGS/Base/Maths.h"

#include <array>

namespace RGS {

    enum class MSAA : int 
    {
        None = 1,
        X2 = 2,
        X3 = 3,
        X4 = 4,
        X5 = 5,
        X6 = 6,
        X7 = 7,
        X8 = 8
    };

    namespace MSAA_SampleTable 
    {
        constexpr std::array<Vec2, 1> Points1x =
        { {
            {0.5f, 0.5f}
        } };


        constexpr std::array<Vec2, 2> Points2x = 
        { {
            {0.25f, 0.25f},
            {0.75f, 0.75f}
        } };

#if 1
        constexpr std::array<Vec2, 3> Points3x = 
        { {
            {0.5f, 0.5f},
            {0.25f, 0.75f},
            {0.75f, 0.25f}
        } };
#else
        constexpr std::array<Vec2, 3> Points3x =
        { {
            {0.5f, 0.2f},  // Bottom center
            {0.2f, 0.8f},  // Top left
            {0.8f, 0.8f}   // Top right
        } };
#endif

#if 0
        constexpr std::array<Vec2, 4> Points4x = 
        { {
            {0.25f, 0.25f},
            {0.75f, 0.25f},
            {0.25f, 0.75f},
            {0.75f, 0.75f}
        } };
#else
        constexpr std::array<Vec2, 4> Points4x =
        { {
            {0.50f, 0.25f },  // 顶部点
            {0.75f, 0.50f },  // 右侧点
            {0.50f, 0.75f },  // 底部点
            {0.25f, 0.50f }   // 左侧点
        } };
#endif

        constexpr std::array<Vec2, 5> Points5x =
        { {
            {0.5f, 0.5f},   // 中心点
            {0.25f, 0.25f}, // 左上
            {0.75f, 0.25f}, // 右上
            {0.25f, 0.75f}, // 左下
            {0.75f, 0.75f}  // 右下
        } };

        constexpr std::array<Vec2, 6> Points6x =
        { {
            {0.5f, 0.5f},   // 中心点
            {0.25f, 0.25f}, // 左上
            {0.75f, 0.25f}, // 右上
            {0.25f, 0.75f}, // 左下
            {0.75f, 0.75f}, // 右下
            {0.5f, 0.25f}   // 顶部中间点
        } };

        constexpr std::array<Vec2, 7> Points7x =
        { {
            {0.5f, 0.5f},   // 中心点
            {0.25f, 0.25f}, // 左上
            {0.75f, 0.25f}, // 右上
            {0.25f, 0.75f}, // 左下
            {0.75f, 0.75f}, // 右下
            {0.5f, 0.25f},  // 顶部中间点
            {0.5f, 0.75f}   // 底部中间点
        } };
        
        constexpr std::array<Vec2, 8> Points8x = 
        { {
            {0.125f, 0.125f},
            {0.375f, 0.125f},
            {0.625f, 0.125f},
            {0.875f, 0.125f},
            {0.125f, 0.375f},
            {0.375f, 0.375f},
            {0.625f, 0.375f},
            {0.875f, 0.375f}
        } };


        template<MSAA msaa>
        constexpr auto GetSamplePoints();

        template<>
        constexpr auto GetSamplePoints<MSAA::None>()
        {
            return Points1x;
        }

        template<>
        constexpr auto GetSamplePoints<MSAA::X2>() 
        {
            return Points2x;
        }

        template<>
        constexpr auto GetSamplePoints<MSAA::X3>() 
        {
            return Points3x;
        }

        template<>
        constexpr auto GetSamplePoints<MSAA::X4>() 
        {
            return Points4x;
        }

        template<>
        constexpr auto GetSamplePoints<MSAA::X5>()
        {
            return Points5x;
        }

        template<>
        constexpr auto GetSamplePoints<MSAA::X6>()
        {
            return Points6x;
        }

        template<>
        constexpr auto GetSamplePoints<MSAA::X7>()
        {
            return Points7x;
        }

        template<>
        constexpr auto GetSamplePoints<MSAA::X8>() 
        {
            return Points8x;
        }
    }
}
