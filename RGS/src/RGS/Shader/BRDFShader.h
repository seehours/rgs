#pragma once
#include "ShaderBase.h"

namespace RGS {

    struct BRDFVertex : public VertexBase
    {
    };

    struct BRDFVaryings : public VaryingsBase
    {
    };

    struct BRDFUniforms : public UniformsBase
    {
    };

  
    void BRDFVertexShader(BRDFVaryings& varyings, const BRDFVertex& vertex, const BRDFUniforms& uniforms);

    Vec4 BRDFFragmentShader(bool& discard, const BRDFVaryings& varyings, const BRDFUniforms& uniforms);

}