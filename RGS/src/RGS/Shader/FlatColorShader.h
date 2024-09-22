#pragma once 
#include "ShaderBase.h"

namespace RGS {

    using FlatColorVertex = VertexBase3D;

    struct FlatColorVaryings : public VaryingsBase {};

    struct FlatColorUniforms : public UniformsBase
    {
        Vec4 Color;
    };

    void FlatColorVertexShader(FlatColorVaryings& varyings, const FlatColorVertex& vertex, const FlatColorUniforms& uniforms);

    Vec4 FlatColorFragmentShader(bool& discard, const FlatColorVaryings& varyings, const FlatColorUniforms& uniforms);

}
