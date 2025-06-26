#pragma once

#include <memory>

#include <SDL3/SDL_gpu.h>

#include "buffer_sdl.h"

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

    void update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t stride) override;

private:
    RendererSDL&               m_renderer;
    std::unique_ptr<BufferSDL> m_buffer = nullptr;
};

}  // namespace storm
