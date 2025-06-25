#pragma once

#include <memory>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/i_buffer.h>

namespace storm
{

class RendererSDL;

class VertexBufferSDL final: virtual public IBuffer
{
public:
    VertexBufferSDL(
        RendererSDL& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, void const* vertex_data, uint32_t vertex_data_size);

    ~VertexBufferSDL() override;

    void bind_to_render_pass() const override;

private:
    RendererSDL& m_renderer;

    std::shared_ptr<SDL_GPUBuffer> m_buffer = nullptr;
};

}  // namespace storm
