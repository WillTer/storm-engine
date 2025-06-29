#pragma once

#include <memory>

#include "gpu_buffer.h"

namespace storm
{

class GPUVertexBuffer final: public GPUBuffer
{
public:
    GPUVertexBuffer(std::shared_ptr<SDL_GPUDevice> const& device, uint32_t vertex_count, uint32_t vertex_type_size);

    ~GPUVertexBuffer();

    void bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const;
};

}  // namespace storm
