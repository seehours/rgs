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
        bool IsAnother = false;
    };

    void BlinnVertexShader(BlinnVaryings& varyings, const BlinnVertex& vertex, const BlinnUniforms& uniforms);

    Vec4 BlinnFragmentShader(bool& discard, const BlinnVaryings& varyings, const BlinnUniforms& uniforms);

}