#pragma once
#include "Renderer.h"
#include "Framebuffer.h"
#include "RGS/Base/Maths.h"
#include "RGS/Shader/ShaderBase.h"

#include <memory>
#include <functional>

namespace RGS {

    class RenderCommand
    {
    public:
        template<typename vertex_t, typename uniforms_t, typename varyings_t>
        static std::unique_ptr<RenderCommand> Draw(Framebuffer& framebuffer,
                                                   std::shared_ptr<Program<vertex_t, uniforms_t, varyings_t>> program,
                                                   std::shared_ptr<Mesh<vertex_t>> mesh,
                                                   std::shared_ptr<uniforms_t> uniforms)
        {

            std::unique_ptr<RenderCommand> command(new RenderCommand());
            command->m_Self = [=, &framebuffer]()
            {
                Renderer::Draw(framebuffer, program, mesh, uniforms);
            };
            return command;
        }

        template<typename vertex_t, typename uniforms_t, typename varyings_t>
        static std::unique_ptr<RenderCommand> Draw(Framebuffer& framebuffer,
                                                   std::shared_ptr<Program<vertex_t, uniforms_t, varyings_t>> program,
                                                   std::shared_ptr<Mesh<vertex_t>> mesh,
                                                   std::shared_ptr<uniforms_t> uniforms,
                                                   const MSAA msaa)
        {
            std::unique_ptr<RenderCommand> command(new RenderCommand());
            command->m_Self = [=, &framebuffer]()
            {
                Renderer::Draw(framebuffer, program, mesh, uniforms, msaa);
            };
            return command;
        }

        static std::unique_ptr<RenderCommand> Clear(Framebuffer& framebuffer, Vec4 color = { 0.0f, 0.0f, 0.0f, 0.0f });
        static std::unique_ptr<RenderCommand> ClearDepth(Framebuffer& framebuffer, float depth = 1.0f);
       
        static std::unique_ptr<RenderCommand> ResolveParallel(Framebuffer& framebuffer, const bool wait = true);
        static std::unique_ptr<RenderCommand> BlitToScreen(Framebuffer& framebuffer);

        void Excecute();
      
    private:
        RenderCommand();

        std::function<void()> m_Self;
    };

}