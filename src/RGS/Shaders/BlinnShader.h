#pragma once
#include "ShaderBase.h"

#include "Math.h"

namespace RGS {

    struct BlinnVertex : public VertexBase
    {
    };

    struct BlinnVaryings : public VaryingsBase
    {
    };

    struct BlinnUniforms : public UniformsBase
    {
    };

    void BlinnVertexShader(BlinnVaryings& varyings, const BlinnVertex& vertex, const BlinnUniforms& uniforms);

}