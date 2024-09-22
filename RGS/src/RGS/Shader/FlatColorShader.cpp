#include "FlatColorShader.h"

namespace RGS {

    void FlatColorVertexShader(FlatColorVaryings& varyings, const FlatColorVertex& vertex, const FlatColorUniforms& uniforms)
    {
        varyings.ClipPos = uniforms.MVP * vertex.ModelPos;
    }

    Vec4 FlatColorFragmentShader(bool& discard, const FlatColorVaryings& varyings, const FlatColorUniforms& uniforms)
    {
        discard = false;
        return uniforms.Color;
    }

}
