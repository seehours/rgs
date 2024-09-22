#include "rgspch.h"
#include "Pipeline.h"
#include "Application.h"

namespace RGS {

    void Pipeline::BeginFrame()
    {
        m_BeginFrameQueue.clear();
        m_GeometryQueue.clear();
        m_TransparentQueue.clear();
        m_EndFrameQueue.clear();
    }

    void Pipeline::EndFrame()
    {
        FlushCommandQueue();
    }

    void Pipeline::AddCommand(std::unique_ptr<RenderCommand> command, RenderStage stage)
    {
        switch (stage)
        {
        case RenderStage::BeginFrame:
            m_BeginFrameQueue.emplace_back(std::move(command));
            break;
        case RenderStage::Geometry:
            m_GeometryQueue.emplace_back(std::move(command));
            break;
        case RenderStage::Transparent:
            m_TransparentQueue.emplace_back(std::move(command));
            break;
        case RenderStage::EndFrame:
            m_EndFrameQueue.emplace_back(std::move(command));
            break;
        default:
            break;
        }
    }

    void Pipeline::FlushCommandQueue()
    {
        {
            RGS_PROFILE_SCOPE("m_BeginFrameQueue");
            for (auto& command : m_BeginFrameQueue)
            {
                command->Excecute();
            }
            m_BeginFrameQueue.clear();
            JobSystem::Wait();
        }

        {
            RGS_PROFILE_SCOPE("m_GeometryQueue");   
            for (auto& command : m_GeometryQueue)
            {
                command->Excecute();
            }
            m_GeometryQueue.clear();
            JobSystem::Wait();
        }

        {
            RGS_PROFILE_SCOPE("m_TransparentQueue");
            for (auto& command : m_TransparentQueue)
            {
                command->Excecute();
                JobSystem::Wait();
            }
            m_TransparentQueue.clear();
        }
        for (auto& command : m_EndFrameQueue)
        {
            command->Excecute();
        }
        m_EndFrameQueue.clear();
    }

}