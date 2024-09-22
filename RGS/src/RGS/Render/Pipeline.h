#pragma once
#include "RenderCommand.h"

#include <vector>
#include <memory>

namespace RGS
{

    enum class RenderStage 
    {
        BeginFrame,
        Geometry,
        Transparent,
        EndFrame,
    };

    class Pipeline
    {
    public:
        void BeginFrame();
        void EndFrame();

        void AddCommand(std::unique_ptr<RenderCommand> command, RenderStage stage);

    private:
        void FlushCommandQueue();

        std::vector<std::unique_ptr<RenderCommand>> m_BeginFrameQueue;
        std::vector<std::unique_ptr<RenderCommand>> m_GeometryQueue;
        std::vector<std::unique_ptr<RenderCommand>> m_TransparentQueue;
        std::vector<std::unique_ptr<RenderCommand>> m_EndFrameQueue;
    };

}
