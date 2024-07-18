#pragma once
#include "RGS/Maths.h"
#include <string>

namespace RGS {

    struct VertexBase
    {
        Vec4 ModelPos = { 0, 0, 0, 1 };
        operator const std::string() const { return "ModelPos: " + (std::string)ModelPos; }
    };

    struct VaryingsBase
    {
        Vec4 ClipPos = { 0, 0, 0, 1 };
    };
 
    struct UniformsBase
    {
        Mat4 MVP;
        operator const std::string() const { return (std::string)MVP; }
    };

}