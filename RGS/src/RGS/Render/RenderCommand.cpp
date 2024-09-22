#include "rgspch.h"
#include "RenderCommand.h"

#include "Application.h"

namespace RGS {
    
    std::unique_ptr<RenderCommand> RenderCommand::Clear(Framebuffer& framebuffer, Vec4 color)
    {
        std::unique_ptr<RenderCommand> command(new RenderCommand());
        command->m_Self = [=, &framebuffer]() 
        {
            framebuffer.Clear(color);
        };
        return command;
    }
    
    std::unique_ptr<RenderCommand> RenderCommand::ClearDepth(Framebuffer& framebuffer, float depth)
    {
        std::unique_ptr<RenderCommand> command(new RenderCommand());
        command->m_Self = [=, &framebuffer]() 
        {
            framebuffer.ClearDepth(depth);
        };
        return command;
    }

    std::unique_ptr<RenderCommand> RenderCommand::ResolveParallel(Framebuffer& framebuffer, const bool wait)
    {
        std::unique_ptr<RenderCommand> command(new RenderCommand());
        command->m_Self = [=, &framebuffer]()
        {
            framebuffer.ResolveParallel();
        };
        return command;
    }

    std::unique_ptr<RenderCommand> RenderCommand::BlitToScreen(Framebuffer& framebuffer)
    {
        std::unique_ptr<RenderCommand> command(new RenderCommand());
        command->m_Self = [=, &framebuffer]()
        {
            Application::Instance().GetFramebuffer().Blit(framebuffer);
        };
        return command;
    }

    void RenderCommand::Excecute()
    {
        m_Self();
    }

    RenderCommand::RenderCommand()
        : m_Self(nullptr) { }
}