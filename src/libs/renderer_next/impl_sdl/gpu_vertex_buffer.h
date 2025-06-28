#pragma once

#include <memory>

#include <SDL3/SDL_gpu.h>

#include "gpu_buffer.h"

namespace storm
{

class RendererService;

class GPUVertexBuffer final
{
public:
    GPUVertexBuffer(
        RendererService& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, void const* vertex_data, uint32_t vertex_data_size);

    ~GPUVertexBuffer();

    void bind_to_render_pass() const;

    void update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t stride);

private:
    RendererService&           m_renderer;
    std::unique_ptr<GPUBuffer> m_buffer = nullptr;
};

}  // namespace storm
