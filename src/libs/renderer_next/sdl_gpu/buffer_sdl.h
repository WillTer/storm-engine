#pragma once

#include <memory>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/i_buffer.h>

namespace storm
{

class RendererSDL;

class BufferSDL final: virtual public IBuffer
{
public:
    explicit BufferSDL(RendererSDL& renderer);
    ~BufferSDL() override;

    void bind_to_render_pass() const override;
    void update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t stride) override;

    void                           set_gpu_buffer(std::shared_ptr<SDL_GPUBuffer> const& buffer);
    std::shared_ptr<SDL_GPUBuffer> get_gpu_buffer() const;

private:
    RendererSDL& m_renderer;

    std::shared_ptr<SDL_GPUBuffer> m_buffer = nullptr;
};

}  // namespace storm
