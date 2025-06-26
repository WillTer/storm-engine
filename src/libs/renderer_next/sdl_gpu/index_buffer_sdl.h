#pragma once

#include <memory>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/i_buffer.h>

#include "buffer_sdl.h"

namespace storm
{

class RendererSDL;

class IndexBufferSDL final: virtual public IIndexBuffer
{
public:
    IndexBufferSDL(RendererSDL& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, std::vector<uint16_t> const& indices);

    ~IndexBufferSDL() override;

    void bind_to_render_pass() const override;

    void draw_indexed() const override;

    void update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t stride) override;

private:
    RendererSDL&               m_renderer;
    std::unique_ptr<BufferSDL> m_buffer = nullptr;

    uint32_t m_index_count = 0;
};

}  // namespace storm
