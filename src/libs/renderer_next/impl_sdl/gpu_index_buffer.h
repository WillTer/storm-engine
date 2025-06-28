#pragma once

#include <memory>
#include <vector>

#include <SDL3/SDL_gpu.h>

#include "gpu_buffer.h"

namespace storm
{

class RendererService;

class GPUIndexBuffer final
{
public:
    GPUIndexBuffer(
        RendererService&                        renderer,
        std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
        void const*                             data,
        uint32_t                                data_size,
        uint32_t                                index_size);

    ~GPUIndexBuffer();

    void bind_to_render_pass() const;

    void draw_indexed() const;

    void update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t stride);

private:
    RendererService&           m_renderer;
    std::unique_ptr<GPUBuffer> m_buffer = nullptr;

    uint32_t                m_index_count = 0;
    SDL_GPUIndexElementSize m_element_size;
};

}  // namespace storm
