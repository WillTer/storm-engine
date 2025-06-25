#pragma once

#include <memory>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/i_buffer.h>

namespace storm
{

class RendererSDL;

class IndexBufferSDL final: virtual public IBuffer
{
public:
    IndexBufferSDL(RendererSDL& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, std::vector<uint16_t> const& indices);

    ~IndexBufferSDL() override;

    void bind_to_render_pass() const override;

private:
    RendererSDL& m_renderer;

    std::shared_ptr<SDL_GPUBuffer> m_buffer = nullptr;

    uint32_t m_index_count = 0;
};

}  // namespace storm
