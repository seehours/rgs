#include "Renderer.h"

namespace RGS {

    bool Renderer::IsVertexVisible(const Vec4& clipPos)
    {
        return fabs(clipPos.X) <= clipPos.W && fabs(clipPos.Y) <= clipPos.W && fabs(clipPos.Z) <= clipPos.W;
    }

    bool Renderer::IsInsidePlane(const Vec4& clipPos, const Plane plane)
    {
        switch (plane)
        {
        case Plane::POSITIVE_W:
            return clipPos.W >= 0.0f;
        case Plane::POSITIVE_X:
            return clipPos.X <= +clipPos.W;
        case Plane::NEGATIVE_X:
            return clipPos.X >= -clipPos.W;
        case Plane::POSITIVE_Y:
            return clipPos.Y <= +clipPos.W;
        case Plane::NEGATIVE_Y:
            return clipPos.Y >= -clipPos.W;
        case Plane::POSITIVE_Z:
            return clipPos.Z <= +clipPos.W;
        case Plane::NEGATIVE_Z:
            return clipPos.Z >= -clipPos.W;
        default:
            ASSERT(false);
            return false;
        }
    }

    bool Renderer::IsInsideTriangle(float(&weights)[3])
    {
        return weights[0] >= -EPSILON && weights[1] >= -EPSILON && weights[2] >= -EPSILON;
    }

    float Renderer::GetIntersectRatio(const Vec4& prev, const Vec4& curr, const Plane plane)
    {
        switch (plane) {
        case Plane::POSITIVE_X:
            return (prev.W - prev.X) / ((prev.W - prev.X) - (curr.W - curr.X));
        case Plane::NEGATIVE_X:
            return (prev.W + prev.X) / ((prev.W + prev.X) - (curr.W + curr.X));
        case Plane::POSITIVE_Y:
            return (prev.W - prev.Y) / ((prev.W - prev.Y) - (curr.W - curr.Y));
        case Plane::NEGATIVE_Y:
            return (prev.W + prev.Y) / ((prev.W + prev.Y) - (curr.W + curr.Y));
        case Plane::POSITIVE_Z:
            return (prev.W - prev.Z) / ((prev.W - prev.Z) - (curr.W - curr.Z));
        case Plane::NEGATIVE_Z:
            return (prev.W + prev.Z) / ((prev.W + prev.Z) - (curr.W + curr.Z));
        default:
            ASSERT(false);
            return 0.0f;
        }
    }

    Renderer::BoundingBox Renderer::GetBoundingBox(const Vec4(&fragCoords)[3],
                                                   const int width,
                                                   const int height)
    {
        auto xList = { fragCoords[0].X, fragCoords[1].X, fragCoords[2].X };
        auto yList = { fragCoords[0].Y, fragCoords[1].Y, fragCoords[2].Y };

        float minX = std::min<float>(xList);
        float maxX = std::max<float>(xList);
        float minY = std::min<float>(yList);
        float maxY = std::max<float>(yList);

        minX = Clamp(minX, 0.0f, (float)(width - 1));
        maxX = Clamp(maxX, 0.0f, (float)(width - 1));
        minY = Clamp(minY, 0.0f, (float)(height - 1));
        maxY = Clamp(maxY, 0.0f, (float)(height - 1));

        BoundingBox bBox;
        bBox.MinX = std::floor(minX);
        bBox.MinY = std::floor(minY);
        bBox.MaxX = std::ceil(maxX);
        bBox.MaxY = std::ceil(maxY);

        return bBox;
    }

    void Renderer::CalculateWeights(float(&screenWeights)[3],
                                    float(&weights)[3],
                                    const Vec4(&fragCoords)[3],
                                    const Vec2& screenPoint)
    {
        Vec2 ab = fragCoords[1] - fragCoords[0];
        Vec2 ac = fragCoords[2] - fragCoords[0];
        Vec2 ap = screenPoint - fragCoords[0];
        float factor = 1.0f / (ab.X * ac.Y - ab.Y * ac.X);
        float s = (ac.Y * ap.X - ac.X * ap.Y) * factor;
        float t = (ab.X * ap.Y - ab.Y * ap.X) * factor;
        screenWeights[0] = 1 - s - t;
        screenWeights[1] = s;
        screenWeights[2] = t;

        float w0 = fragCoords[0].W * screenWeights[0];
        float w1 = fragCoords[1].W * screenWeights[1];
        float w2 = fragCoords[2].W * screenWeights[2];
        float normalizer = 1.0f / (w0 + w1 + w2);
        weights[0] = w0 * normalizer;
        weights[1] = w1 * normalizer;
        weights[2] = w2 * normalizer;
    }
}
