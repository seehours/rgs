#include "BlinnShader.h"

namespace RGS {

    void BlinnVertexShader(BlinnVaryings& varyings, const BlinnVertex& vertex, const BlinnUniforms& uniforms)
    {
        varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
    }

    Vec4 BlinnFragmentShader(bool& discard, const BlinnVaryings& varyings, const BlinnUniforms& uniforms)
    {
        int x = varyings.FragPos.X;
        if (x % 10 < 5)
        {
            discard = true;
            return { 0.0f, 0.0f, 0.0f, 1.0f };
        }
        return {  0.0f, varyings.NdcPos.X / 2 + 0.5f, varyings.NdcPos.Y / 2 + 0.5f, 1.0f };
    }
}