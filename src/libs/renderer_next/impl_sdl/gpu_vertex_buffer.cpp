#include "gpu_vertex_buffer.h"

#include <SDL3/SDL_gpu.h>
#include <libs/core/core.h>
#include <spdlog/spdlog.h>

using namespace storm;

GPUVertexBuffer::GPUVertexBuffer(
    std::shared_ptr<SDL_GPUDevice> const& device, void const* const vertices, uint32_t const vertices_data_size)
    : GPUBuffer(device, vertices, vertices_data_size)
{
    auto vertex_buffer_create_info  = SDL_GPUBufferCreateInfo {};
    vertex_buffer_create_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertex_buffer_create_info.size  = vertices_data_size;

    m_buffer = std::shared_ptr<SDL_GPUBuffer>(SDL_CreateGPUBuffer(device.get(), &vertex_buffer_create_info), [device](SDL_GPUBuffer* p) {
        SDL_ReleaseGPUBuffer(device.get(), p);
    });

    if (!m_buffer) { throw std::runtime_error(std::format("Failed to create vertex buffer: {}", SDL_GetError())); }
}

GPUVertexBuffer::~GPUVertexBuffer() = default;

void GPUVertexBuffer::bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const
{
    assert(render_pass);

    auto vertex_binding   = SDL_GPUBufferBinding {};
    vertex_binding.buffer = m_buffer.get();
    vertex_binding.offset = 0;
    SDL_BindGPUVertexBuffers(render_pass.get(), 0, &vertex_binding, 1);
}
