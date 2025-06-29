#pragma once

#include <memory>

#include "gpu_buffer.h"
#include "sdl_fwd.h"

namespace storm
{

class GPUIndexBuffer final: public GPUBuffer
{
public:
    GPUIndexBuffer(std::shared_ptr<SDL_GPUDevice> const& device, uint32_t index_count);

    ~GPUIndexBuffer();

    void bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const;
    void draw(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const;

private:
    uint32_t m_index_count = 0;
};

}  // namespace storm
