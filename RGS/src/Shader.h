#pragma once
#include "Base.h"
#include "Layouts.h"
#include "Math.h"

namespace RGS {

    void SimpleVertexShader(SimpleVaryings& varyings, const SimpleVertex& vertex, const SimpleUniforms& uniforms);
    void BlinnVertexShader(BlinnVaryings& varyings, const BlinnVertex& vertex, const BlinnUniforms& uniforms);

    Vec4 SimpleFragmentShader(bool& discard, const SimpleVaryings& varyings, const SimpleUniforms& uniforms);
    Vec4 SimpleColorFragmentShader(bool& discard, const SimpleVaryings& varyings, const SimpleUniforms& uniforms);
    Vec4 BlinnFragmentShader(bool& discard, const BlinnVaryings& varyings, const BlinnUniforms& uniforms);

}


