#include "Math.h"

#include "Base.h"
#include <math.h>

namespace RGS {

    float Dot(const Vec3& left, const Vec3& right)
    {
        return left.X * right.X + left.Y * right.Y + left.Z * right.Z;
    }

    Mat4::Mat4(const Vec4& v0, const Vec4& v1, const Vec4& v2, const Vec4& v3)
    {
        M[0][0] = v0.X; M[1][0] = v0.Y; M[2][0] = v0.Z; M[3][0] = v0.W;
        M[0][1] = v1.X; M[1][1] = v1.Y; M[2][1] = v1.Z; M[3][1] = v1.W;
        M[0][2] = v2.X; M[1][2] = v2.Y; M[2][2] = v2.Z; M[3][2] = v2.W;
        M[0][3] = v3.X; M[1][3] = v3.Y; M[2][3] = v3.Z; M[3][3] = v3.W;
    }

    Vec4 operator* (const Mat4& mat4, const Vec4& vec4)
    {
        Vec4 res;
        res.X = mat4.M[0][0] * vec4.X + mat4.M[0][1] * vec4.Y + mat4.M[0][2] * vec4.Z + mat4.M[0][3] * vec4.W;
        res.Y = mat4.M[1][0] * vec4.X + mat4.M[1][1] * vec4.Y + mat4.M[1][2] * vec4.Z + mat4.M[1][3] * vec4.W;
        res.Z = mat4.M[2][0] * vec4.X + mat4.M[2][1] * vec4.Y + mat4.M[2][2] * vec4.Z + mat4.M[2][3] * vec4.W;
        res.W = mat4.M[3][0] * vec4.X + mat4.M[3][1] * vec4.Y + mat4.M[3][2] * vec4.Z + mat4.M[3][3] * vec4.W;
        return res;
    }
    Mat4 operator* (const Mat4& left, const Mat4& right)
    {
        Mat4 res;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    res.M[i][j] += left.M[i][k] * right.M[k][j];
                }
            }
        }
        return res;
    }
    Mat4& operator*= (Mat4& left, const Mat4& right)
    {
        left = left * right;
        return left;
    }

    Mat4 Mat4Identity()
    {
        return Mat4({ 1,0,0,0 }, { 0,1,0,0 }, { 0,0,1,0 }, { 0,0,0,1 });
    }
    Mat4 Mat4Translate(float tx, float ty, float tz)
    {
        Mat4 m = Mat4Identity();
        m.M[0][3] = tx;
        m.M[1][3] = ty;
        m.M[2][3] = tz;
        return m;
    }
    Mat4 Mat4Scale(float sx, float sy, float sz)
    {
        Mat4 m = Mat4Identity();
        ASSERT(sx != 0 && sy != 0 && sz != 0);
        m.M[0][0] = sx;
        m.M[1][1] = sy;
        m.M[2][2] = sz;
        return m;
    }
    Mat4 Mat4RotateX(float angle)
    {
        float c = (float)cos(angle);
        float s = (float)sin(angle);
        Mat4 m = Mat4Identity();
        m.M[1][1] = c;
        m.M[1][2] = -s;
        m.M[2][1] = s;
        m.M[2][2] = c;
        return m;
    }
    Mat4 Mat4RotateY(float angle)
    {
        float c = (float)cos(angle);
        float s = (float)sin(angle);
        Mat4 m = Mat4Identity();
        m.M[2][2] = c;
        m.M[2][0] = -s;
        m.M[0][2] = s;
        m.M[0][0] = c;
        return m;
    }
    Mat4 Mat4RotateZ(float angle)
    {
        float c = (float)cos(angle);
        float s = (float)sin(angle);
        Mat4 m = Mat4Identity();
        m.M[0][0] = c;
        m.M[0][1] = -s;
        m.M[1][0] = s;
        m.M[1][1] = c;
        return m;
    }
    Mat4 Mat4LookAt(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis, const Vec3& eye)
    {
        Mat4 m = Mat4Identity();

        m.M[0][0] = xAxis.X;
        m.M[0][1] = xAxis.Y;
        m.M[0][2] = xAxis.Z;

        m.M[1][0] = yAxis.X;
        m.M[1][1] = yAxis.Y;
        m.M[1][2] = yAxis.Z;

        m.M[2][0] = zAxis.X;
        m.M[2][1] = zAxis.Y;
        m.M[2][2] = zAxis.Z;

        m.M[0][3] = -Dot(xAxis, eye);
        m.M[1][3] = -Dot(yAxis, eye);
        m.M[2][3] = -Dot(zAxis, eye);

        return m;
    }

    unsigned char Float2UChar(const float f)
    {
        return (unsigned char)(f * 255.0f);
    }
    float UChar2Float(const unsigned char c)
    {
        return (float)c / 255.0f;
    }
}