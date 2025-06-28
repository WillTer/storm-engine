#pragma once

#include <memory>
#include <vector>

#include <SDL3/SDL_gpu.h>

namespace storm
{

class RendererService;

struct BufferUpdateInfo {
    uint32_t offset;
    uint32_t size;
};

class GPUBuffer final
{
public:
    explicit GPUBuffer(RendererService& renderer);
    ~GPUBuffer();

    void bind_to_render_pass() const;
    void update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t stride);

    void                           set_gpu_buffer(std::shared_ptr<SDL_GPUBuffer> const& buffer);
    std::shared_ptr<SDL_GPUBuffer> get_gpu_buffer() const;

private:
    RendererService& m_renderer;

    std::shared_ptr<SDL_GPUBuffer> m_buffer = nullptr;
};

}  // namespace storm
